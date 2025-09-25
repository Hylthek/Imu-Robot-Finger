close all;
format compact
clc

% Plot several views of TestImuData.csv (IMU time series)

fname = 'LooseGripZTaps.csv';

set(0, 'DefaultFigureWindowStyle', 'normal');
set(0, 'DefaultFigurePosition', get(0, 'ScreenSize'));

%% Load
opts = detectImportOptions(fname,'NumHeaderLines',0,'VariableNamingRule','preserve');
T = readtable(fname, opts);

% Columns by index (avoids header quirks)
t_us   = T{:,1};  % Time (microseconds)
ax_raw = T{:,2};  % Accel X (raw)
ay_raw = T{:,3};  % Accel Y
az_raw = T{:,4};  % Accel Z
gx_raw = T{:,5};  % Gyro X (raw)
gy_raw = T{:,6};  % Gyro Y
gz_raw = T{:,7};  % Gyro Z











%% Convert units
t = (t_us - t_us(1)) * 1e-6;   % seconds from start
% rebases the time axis, converts units to seconds
ax = ax_raw / 2048;            % G
ay = ay_raw / 2048;            % G
az = az_raw / 2048;            % G
ax = ax_raw / 2048;            % G
ay = ay_raw / 2048;            % G
az = az_raw / 2048;            % G
gx = gx_raw / 65.5;            % deg/s
gy = gy_raw / 65.5;            % deg/s
gz = gz_raw / 65.5;            % deg/s
a_mag = sqrt(ax.^2 + ay.^2 + az.^2);









%% Sampling rate (use median to be robust)
dt = median(diff(t));
% take differences between samples, take the median, result is sampling period
fs = 1/dt;
% take the inverse of the period to get sampling frequency

fprintf('Samples: %d | Duration: %.2f s | fs ≈ %.2f Hz\n', numel(t), t(end)-t(1), fs);

dt_all = diff(t);
fprintf('dt min: %.6f s | dt max: %.6f s\n', min(dt_all), max(dt_all));





dt_all = diff(t);
fprintf('dt min: %.6f s | dt max: %.6f s\n', min(dt_all), max(dt_all));





%% Figure 1: Acceleration (3-axis)
figure('Name', 'Acceleration (G) vs Time ', 'Color', 'w');
tabgp = uitabgroup;

tab1 = uitab(tabgp, 'Title', 'A_x');
axes('Parent', tab1);
plot(t, ax, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('A_x (G)'); title('Acceleration');
ylim([-16 16]);

tab2 = uitab(tabgp, 'Title', 'A_y');
axes('Parent', tab2);
plot(t, ay, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('A_y (G)');
ylim([-16 16]);

tab3 = uitab(tabgp, 'Title', 'A_z');
axes('Parent', tab3);
plot(t, az, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('A_z (G)'); xlabel('Time (s)');
ylim([-16 16]);

tab4 = uitab(tabgp, 'Title', 'All Axes');
axes('Parent', tab4);
plot(t, ax, 'r', t, ay, 'g', t, az, 'b', 'LineWidth',1); grid on;
ylabel('Accel (G)');
xlabel('Time (s)');
legend('A_x','A_y','A_z');
title('All Axes');
ylim([-16 16]);

return


%% Figure 2: Gyroscope (3-axis)
figure('Name','Gyroscope (deg/s) vs Time','Color','w');
tiledlayout(3,1,'Padding','compact','TileSpacing','compact');
nexttile; plot(t, gx, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_x (deg/s)'); title('Gyroscope');
nexttile; plot(t, gy, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_y (deg/s)');
nexttile; plot(t, gz, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_z (deg/s)'); xlabel('Time (s)');


return

nexttile; plot(t, gx, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_x (deg/s)'); title('Gyroscope');
nexttile; plot(t, gy, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_y (deg/s)');
nexttile; plot(t, gz, 'LineWidth',1, 'Marker', '.'); grid on; ylabel('G_z (deg/s)'); xlabel('Time (s)');


return


%% Figure 3: Spectrograms (Acceleration) — no local functions (mlx-friendly)
% Windowing params (tune as needed)
window_length_Sec = 0.25;                                        % 250 ms
window_length_Samples = max(64, min(numel(t), round(window_length_Sec*fs)));    % cap at signal length
number_of_points_overlap  = round(0.5*window_length_Samples);
number_of_points_fft      = 2^nextpow2(max(window_length_Samples, 256));
% ensures number of points in FFT at least 256
window = hamming(window_length_Samples, 'periodic');

figure('Name','Acceleration Spectrograms','Color','w');
tiledlayout(2,2,'Padding','compact','TileSpacing','compact');

nexttile;
spectrogram(ax - mean(ax,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Ax Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(ay - mean(ay,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Ay Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(az - mean(az,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Az Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(a_mag - mean(a_mag,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('|a| Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

%% Figure 4: Spectrograms (Gyroscope)
% Ensure gyro magnitude exists
g_mag = sqrt(gx.^2 + gy.^2 + gz.^2);

% Reuse window params from accel section (win, Noverlap, Nfft, fs)
figure('Name','Gyroscope Spectrograms','Color','w');
tiledlayout(2,2,'Padding','compact','TileSpacing','compact');

nexttile;
spectrogram(gx - mean(gx,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Gx Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(gy - mean(gy,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Gy Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(gz - mean(gz,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Gz Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;

nexttile;
spectrogram(g_mag - mean(g_mag,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('|g| Spectrogram'); colorbar; ylim([0, min(4000, fs/2)]); grid on;


%% Figure 4: Histograms
numBinsA = 100; numBinsG = 100;
figure('Name','Histograms','Color','w');
tiledlayout(2,3,'Padding','compact','TileSpacing','compact');
nexttile; histogram(ax, numBinsA); grid on; title('A_x (G)');
nexttile; histogram(ay, numBinsA); grid on; title('A_y (G)');
nexttile; histogram(az, numBinsA); grid on; title('A_z (G)');
nexttile; histogram(gx, numBinsG); grid on; title('G_x (deg/s)');
nexttile; histogram(gy, numBinsG); grid on; title('G_y (deg/s)');
nexttile; histogram(gz, numBinsG); grid on; title('G_z (deg/s)');

%% Figure 5: Power Spectral Density (Welch)
if fs > 5
    nseg = min( floor(numel(t)/8) , 4096 );
    nseg = max(nseg, 64);                 % ensure enough points
    wwel = hamming(nseg);
    nover = floor(0.5*nseg);
    
    
    figure('Name','PSD - Acceleration','Color','w');
    tiledlayout(3,1,'Padding','compact','TileSpacing','compact');
    nexttile; pwelch(ax, wwel, nover, [], fs, 'onesided'); grid on; title('A_x PSD'); ylabel('Power/Hz');
    nexttile; pwelch(ay, wwel, nover, [], fs, 'onesided'); grid on; title('A_y PSD'); ylabel('Power/Hz');
    nexttile; pwelch(az, wwel, nover, [], fs, 'onesided'); grid on; title('A_z PSD'); xlabel('Hz'); ylabel('Power/Hz');
    
    
    figure('Name','PSD - Gyroscope','Color','w');
    tiledlayout(3,1,'Padding','compact','TileSpacing','compact');
    nexttile; pwelch(gx, wwel, nover, [], fs, 'onesided'); grid on; title('G_x PSD'); ylabel('Power/Hz');
    nexttile; pwelch(gy, wwel, nover, [], fs, 'onesided'); grid on; title('G_y PSD'); ylabel('Power/Hz');
    nexttile; pwelch(gz, wwel, nover, [], fs, 'onesided'); grid on; title('G_z PSD'); xlabel('Hz'); ylabel('Power/Hz');
else
    warning('Sampling rate (%.2f Hz) too low for meaningful PSD; skipping.', fs);
end

%% Optional: save figures as PNG
% figlist = findall(0,'Type','figure');
% for k = 1:numel(figlist)
%     f = figlist(k);
%     nm = get(f,'Name'); if isempty(nm), nm = sprintf('Figure_%d', k); end
%     exportgraphics(f, [regexprep(nm,'\s+','_'), '.png'], 'Resolution',150);
% end
