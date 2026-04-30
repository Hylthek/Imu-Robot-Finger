function resultsSummary = testModels(TestDataStats)
    % Evaluates all saved models against the test dataset and plots confusion matrices.
    
    modelFiles = {
        'Model_BaggedTrees.mat',
        'Model_CubicSVM.mat',
        'Model_MediumGaussianSVM.mat',
        'Model_MediumNN.mat',
        'Model_QuadraticSVM.mat',
        'Model_SVM_Kernel.mat',
        'Model_WideNN.mat'
    };
    
    resultsSummary = table('Size', [length(modelFiles), 2], ...
        'VariableTypes', {'string', 'double'}, ...
        'VariableNames', {'ModelName', 'TestAccuracy'});
        
    trueLabels = TestDataStats.Material;
    
    for i = 1:length(modelFiles)
        currentModelFile = modelFiles{i};
        
        % --- FIX: Create the full path to the Models folder ---
        fullFilePath = fullfile('Models', currentModelFile);
        
        % --- FIX: Check the full path ---
        if ~isfile(fullFilePath)
            warning('File %s not found in Models folder. Skipping...', currentModelFile);
            continue;
        end
        
        % --- FIX: Load from the full path ---
        load(fullFilePath, 'trainedModel');
        
        predictedLabels = trainedModel.predictFcn(TestDataStats);
        
        isCorrect = (predictedLabels == trueLabels);
        numCorrect = sum(isCorrect);
        totalSamples = length(isCorrect);
        testAccuracy = numCorrect / totalSamples;
        
        modelNameClean = strrep(strrep(currentModelFile, 'Model_', ''), '.mat', '');
        resultsSummary.ModelName(i) = string(strrep(modelNameClean, '_', ' '));
        resultsSummary.TestAccuracy(i) = testAccuracy * 100;
        
        figure('Name', char("Evaluation: " + modelNameClean));
        cc = confusionchart(trueLabels, predictedLabels);
        cc.Title = sprintf('%s - Test Accuracy: %.1f%%', strrep(modelNameClean, '_', ' '), testAccuracy*100);
        cc.RowSummary = 'row-normalized';     
        cc.ColumnSummary = 'column-normalized'; 
    end
    
    % Sort results
    resultsSummary = sortrows(resultsSummary, 'TestAccuracy', 'descend');
    
    fprintf('\n=================================\n');
    fprintf('    FINAL EVALUATION SUMMARY\n');
    fprintf('=================================\n');
    disp(resultsSummary);
    fprintf('=================================\n');
end