function generateAllCCode()
    % Automates the creation of C/C++ static libraries for all trained IMU models.
    % Routes outputs to organized subfolders and cleans up temporary files.
    
    fprintf('--- Starting Automated C-Code Generation ---\n');
    
    % --- NEW: Start global stopwatch ---
    totalTimeStart = tic; 
    
    % 1. Define the models to process
    modelFiles = {
        'Model_BaggedTrees.mat',
        'Model_CubicSVM.mat',
        'Model_MediumGaussianSVM.mat',
        'Model_MediumNN.mat',
        'Model_QuadraticSVM.mat',
        'Model_SVM_Kernel.mat',
        'Model_WideNN.mat'
    };
    % 2. Define the input data shape (1x64 array of doubles)
    inputDef = coder.typeof(0, [1 64], [0 0]);
    
    % --- Define the Master Output Directory ---
    masterOutFolder = 'C_Code';
    if ~exist(masterOutFolder, 'dir')
        mkdir(masterOutFolder);
    end
    
    for i = 1:length(modelFiles)
        currentFile = modelFiles{i};
        fullFilePath = fullfile('Models', currentFile);
        modelNameClean = strrep(strrep(currentFile, 'Model_', ''), '.mat', '');
        
        if ~isfile(fullFilePath)
            warning('File %s not found in Models directory. Skipping...', currentFile);
            continue;
        end
        
        % --- Define the specific subfolder for this model ---
        targetSubFolder = fullfile(masterOutFolder, modelNameClean);
        
        fprintf('\nProcessing %s...\n', modelNameClean);
        
        % --- STEP A: Load and Extract the Raw Model ---
        loadedData = load(fullFilePath);
        trainedModel = loadedData.trainedModel;
        
        fields = fieldnames(trainedModel);
        rawModel = [];
        for f = 1:length(fields)
            if contains(fields{f}, 'Classification')
                rawModel = trainedModel.(fields{f});
                break;
            end
        end
        
        if isempty(rawModel)
            warning('Could not find raw Classification model in %s. Skipping...', currentFile);
            continue;
        end
        
        % --- STEP B: Save for Coder ---
        embedFile = sprintf('Embedded_%s.mat', modelNameClean);
        saveLearnerForCoder(rawModel, embedFile);
        
        % --- STEP C: Programmatically Write the Entry-Point Function ---
        entryFuncName = sprintf('predict_%s', modelNameClean);
        fid = fopen([entryFuncName, '.m'], 'w');
        
        fprintf(fid, 'function label = %s(featureArray)\n', entryFuncName);
        fprintf(fid, '%%#codegen\n');
        fprintf(fid, '%% Auto-generated entry point for %s\n', modelNameClean);
        fprintf(fid, 'mdl = loadLearnerForCoder(''%s'');\n', embedFile);
        fprintf(fid, 'label = predict(mdl, featureArray);\n');
        fprintf(fid, 'end\n');
        fclose(fid);
        
        % --- STEP D: Trigger the MATLAB Coder Compiler ---
        fprintf('  -> Compiling %s.c...\n', entryFuncName);
        try
            % Execute codegen with the -d flag to route output to the subfolder
            evalStr = sprintf('codegen %s -args {inputDef} -config:lib -d ''%s'' -report', entryFuncName, targetSubFolder);
            
            % --- NEW: Start local stopwatch for compilation ---
            modelTimeStart = tic; 
            
            eval(evalStr);
            
            % --- NEW: Stop local stopwatch and read elapsed time ---
            compilationTime = toc(modelTimeStart); 
            
            fprintf('  -> Success! Library generated in /%s (Took %.2f seconds)\n', strrep(targetSubFolder, '\', '/'), compilationTime);
        catch ME
            fprintf('  -> FAILED to compile %s. Error: %s\n', entryFuncName, ME.message);
        end
        
        % --- STEP E: Workspace Cleanup ---
        % Delete the temporary files so your main folder stays clean
        if isfile([entryFuncName, '.m'])
            delete([entryFuncName, '.m']);
        end
        if isfile(embedFile)
            delete(embedFile);
        end
    end
    
    % --- NEW: Stop global stopwatch and print total time ---
    totalTime = toc(totalTimeStart);
    fprintf('\n--- C-Code Generation Complete (Total Time: %.2f seconds) ---\n', totalTime);
end