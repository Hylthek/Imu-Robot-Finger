function trainModels(TrainDataStats)
    % Trains all 7 models using the provided training data and saves them to disk.
    
    % --- Create the Models directory if it doesn't exist ---
    outputFolder = 'Models';
    if ~exist(outputFolder, 'dir')
        mkdir(outputFolder);
        fprintf('Created new directory: %s\n', outputFolder);
    end
    
    functionsToRun = {
        @trainBaggedTrees, 'Model_BaggedTrees.mat';
        @trainCubicSVM, 'Model_CubicSVM.mat';
        @trainMediumGaussianSVM, 'Model_MediumGaussianSVM.mat';
        @trainMediumNN, 'Model_MediumNN.mat';
        @trainQuadraticSVM, 'Model_QuadraticSVM.mat';
        @trainSVM_Kernel, 'Model_SVM_Kernel.mat';
        @trainWideNN, 'Model_WideNN.mat'
    };
    
    for i = 1:size(functionsToRun, 1)
        trainFunc = functionsToRun{i, 1};
        fileName = functionsToRun{i, 2};
        
        % --- Build the full path to the Models folder ---
        savePath = fullfile(outputFolder, fileName);
        
        fprintf('Training %s...\n', func2str(trainFunc));
        
        % Train the model using the data passed into the function
        [trainedModel, valAcc] = trainFunc(TrainDataStats);
        fprintf('  -> Validation Accuracy: %.2f%%\n', valAcc * 100);
        
        % --- Save to the new folder AND include valAcc ---
        save(savePath, 'trainedModel', 'valAcc');
        fprintf('  -> Saved to %s\n', savePath);
    end
end