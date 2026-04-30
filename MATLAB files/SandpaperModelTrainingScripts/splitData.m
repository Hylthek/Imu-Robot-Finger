function [trainData, testData] = splitDataset(fullData, testRatio)
    % Splits data into training and testing sets using a stratified split.
    
    rng('default'); % Ensure reproducible splits
    
    cvp = cvpartition(fullData.Material, 'HoldOut', testRatio);
    
    idxTrain = training(cvp); 
    idxTest = test(cvp);      
    
    trainData = fullData(idxTrain, :);
    testData = fullData(idxTest, :);
    
    fprintf('Training Data : %d windows (%.1f%%)\n', height(trainData), (height(trainData)/cvp.NumObservations)*100);
    fprintf('Testing Data  : %d windows (%.1f%%)\n', height(testData), (height(testData)/cvp.NumObservations)*100);
    
end