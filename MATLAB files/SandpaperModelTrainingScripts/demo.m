%% Pipeline: IMU Material Classification
clear; clc; close all;

% Step 1: Build the full dataset from CSV files
fprintf('--- STEP 1: Processing Data ---\n');
fullDataset = processData();

% Step 2: Split the data into Training (80%) and Testing (20%)
fprintf('\n--- STEP 2: Splitting Data ---\n');
testRatio = 0.2; 
[TrainDataStats, TestDataStats] = splitData(fullDataset, testRatio);

writetable(TestDataStats, 'test_data.csv');

% Step 3: Train all models using the Training set
fprintf('\n--- STEP 3: Training Models ---\n');
trainModels(TrainDataStats);

% Step 4: Evaluate all models using the Testing set
fprintf('\n--- STEP 4: Evaluating Models ---\n');
resultsTable = testModels(TestDataStats);