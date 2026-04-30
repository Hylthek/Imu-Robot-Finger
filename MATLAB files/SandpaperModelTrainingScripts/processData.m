function fullDataStats = processData()
    % Automatically detects CSVs, processes IMU data, and extracts 64 features.
    
%% 1) Paths (Auto-detect directory level)
    if false
        DataRoot = pwd; 
    elseif isfolder(fullfile(pwd, 'Data')) && ~isempty(dir(fullfile(pwd, 'Data', '*.csv')))
        % --- Now looks inside the 'Data' folder ---
        DataRoot = fullfile(pwd, 'Data'); 
    else
        error("Could not locate the CSV files. Ensure they are in a folder named 'Data'.");
    end
    
    files = dir(fullfile(DataRoot, '*.csv'));
    fprintf("Found %d CSV files in %s\n", numel(files), DataRoot);

    %% 2) Parameters
    targetFs = 1000;         
    TimeWindow_ms = 200;     
    blocksize = round((TimeWindow_ms/1000) * targetFs); 
    Hop_ms = 50;             
    hop = round((Hop_ms/1000) * targetFs); 
    
    %% 3) Build feature table across all files
    allStats = table();
    for i = 1:numel(files)
        fp = fullfile(files(i).folder, files(i).name);
        
        [~, nameOnly, ~] = fileparts(files(i).name); 
        parts = split(nameOnly, '-'); 
        if numel(parts) >= 2
            materialName = parts{2} + " Grit"; 
        else
            materialName = string(nameOnly); 
        end
        label = categorical(materialName);        

        M = readmatrix(fp);  
        if size(M,2) < 7; continue; end
        
        rawTime = M(:,1);
        rawIMU  = M(:,2:7);
        
        good = ~isnan(rawTime);
        rawTime = rawTime(good);
        rawIMU  = rawIMU(good,:);
        
        clean = preprocessIMU(rawTime, rawIMU, targetFs, label);
        statsT = extractIMUFeatures(clean, blocksize, hop, targetFs);
        
        if ~isempty(statsT)
            allStats = [allStats; statsT]; %#ok<AGROW>
            fprintf("Processed: %s (Yielded %d windows)\n", files(i).name, height(statsT));
        end
    end
    
    fullDataStats = allStats; % Pass the table to the function output
    fprintf("Final Dataset: %d windows, %d columns\n", height(fullDataStats), width(fullDataStats));
end

%% ---------------- Helper Functions ----------------
function cleanTable = preprocessIMU(rawTime, rawIMU, targetFs, label)
    rawTime = rawTime(:);
    
    % Ensure strictly increasing time
    [rawTime, ia] = unique(rawTime);
    rawIMU = rawIMU(ia,:);
    
    dtRaw = median(diff(rawTime), "omitnan");
    
    if dtRaw > 10
        t = (rawTime - rawTime(1)) / 1e6; 
    elseif dtRaw > 0.01
        t = (rawTime - rawTime(1)) / 1e3; 
    else
        t = (rawTime - rawTime(1));       
    end
    
    rawIMU = fillmissing(rawIMU, "linear");
    
    if exist("resample", "file") == 2
        [resampledIMU, ~] = resample(rawIMU, t, targetFs);
    else
        tUniform = (0 : 1/targetFs : t(end)).';
        resampledIMU = interp1(t, rawIMU, tUniform, "linear", "extrap");
    end
    
    % --- Robust Gravity Removal (Set to optimal 2.0 Hz for hand collection) ---
    if exist('highpass', 'file') == 2
        cleanSignals = highpass(resampledIMU, 2.0, targetFs); 
    else
        gravity = movmean(resampledIMU, round(0.5 * targetFs), 1);
        cleanSignals = resampledIMU - gravity;
    end
    
    % --- Delete the Filter Startup Transient ---
    cropIdx = round(0.5 * targetFs);
    if size(cleanSignals, 1) > cropIdx
        cleanSignals = cleanSignals(cropIdx+1:end, :);
    end
    
    cleanTable = array2table(cleanSignals, 'VariableNames', {'Ax','Ay','Az','Gx','Gy','Gz'});
    cleanTable.Material = repmat(label, height(cleanTable), 1);
end

function StatsTable = extractIMUFeatures(Data, blocksize, hop, Fs)
    numericData = Data{:, 1:6};
    
    smv_acc = sqrt(sum(numericData(:,1:3).^2, 2));
    smv_gyr = sqrt(sum(numericData(:,4:6).^2, 2));
    
    % Combine the 6 raw axes AND the 2 SMV vectors into an 8-column matrix
    X = [numericData(:,1:3), smv_acc, numericData(:,4:6), smv_gyr]; 
    N_total = size(X,1);
    
    if N_total < blocksize
        StatsTable = table();
        return;
    end
    
    nWins = floor((N_total - blocksize) / hop) + 1;
    % 8 signals * 8 features = 64 feature columns
    stats = zeros(nWins, 64); 
    
    f = Fs * (0:floor(blocksize/2)) / blocksize;
    keepWindow = false(nWins, 1);
    
    for w = 1:nWins
        startIdx = (w-1)*hop + 1;
        idx = startIdx : (startIdx + blocksize - 1);
        seg = X(idx, :); 
        
        keepWindow(w) = true;
                        
        % =========================================================
        % STEP 3: FEATURE EXTRACTION (On the Normalized Data)
        % =========================================================
        % --- Time Domain ---
        mu   = mean(seg, 1);
        va   = var(seg, 0, 1);
        rmsv = sqrt(mean(seg.^2, 1));
        rngv = max(seg, [], 1) - min(seg, [], 1);
        
        % --- Frequency Domain ---
        Y = fft(seg);
        P2 = abs(Y / blocksize);
        P1 = P2(1:floor(blocksize/2)+1, :);
        P1(2:end-1, :) = 2 * P1(2:end-1, :);
        
        % 1. Peak Frequency
        [maxP, maxIdx] = max(P1, [], 1);
        peakFreq = f(maxIdx);
        
        % 2. Mean Spectral Power
        meanPower = mean(P1.^2, 1);
        
        % 3. Peak Spectral Power
        peakPower = maxP.^2;
        
        % 4. Spectral Entropy (How "noisy/chaotic" the vibration is)
        powerSum = sum(P1.^2, 1);
        powerSum(powerSum == 0) = eps; % Prevent divide-by-zero crashes
        prob = (P1.^2) ./ powerSum;
        prob(prob == 0) = eps;         % Prevent log(0) crashes
        specEnt = -sum(prob .* log2(prob), 1);
        
        % Combine all 8 math features for all 8 signals
        M = [mu(:) va(:) rmsv(:) rngv(:) peakFreq(:) meanPower(:) peakPower(:) specEnt(:)];  
        stats(w,:) = reshape(M.', 1, []);   
    end
    
    stats = stats(keepWindow, :);
    
    if isempty(stats)
        StatsTable = table();
        return;
    end
    
    % Name the 64 columns
    baseSignals = {'Ax','Ay','Az','SMV_Acc','Gx','Gy','Gz','SMV_Gyr'};
    feats       = {'mean','var','rms','range','peakFreq','meanPower','peakPower','specEnt'};
    
    colNames = cell(1, 64);
    k = 1;
    for i = 1:numel(baseSignals)
        for j = 1:numel(feats)
            colNames{k} = sprintf('%s_%s', baseSignals{i}, feats{j});
            k = k + 1;
        end
    end
    
    StatsTable = array2table(stats, 'VariableNames', colNames);
    StatsTable.Material = repmat(Data.Material(1), height(StatsTable), 1);
end