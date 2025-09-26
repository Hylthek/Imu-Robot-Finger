close all;
format compact;
clc;

% Plot several views of TestImuData.csv (IMU time series)

fname = 'Shirt.csv';
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

%% Figure 1: Acceleration (3-axis)
figure('Name', [fname ' - Acceleration (3-axis)'], 'Color', 'w', 'WindowStyle', 'docked');
tgroup = uitabgroup;

tab1 = uitab(tgroup, 'Title', 'A_x');
ax1 = axes(tab1);
plot(ax1, t, ax, 'LineWidth',1, 'Marker', '.'); grid(ax1, 'on');
ylabel(ax1, 'A_x (G)');
title(ax1, 'A_x'); ylim(ax1, [-16 16]);

tab2 = uitab(tgroup, 'Title', 'A_y');
ax2 = axes(tab2);
plot(ax2, t, ay, 'LineWidth',1, 'Marker', '.'); grid(ax2, 'on');
ylabel(ax2, 'A_y (G)');
title(ax2, 'A_y'); ylim(ax2, [-16 16]);

tab3 = uitab(tgroup, 'Title', 'A_z');
ax3 = axes(tab3);
plot(ax3, t, az, 'LineWidth',1, 'Marker', '.'); grid(ax3, 'on');
ylabel(ax3, 'A_z (G)'); xlabel(ax3, 'Time (s)');
title(ax3, 'A_z'); ylim(ax3, [-16 16]);

tab4 = uitab(tgroup, 'Title', 'All Axes');
ax4 = axes(tab4);
plot(ax4, t, ax, 'r', t, ay, 'g', t, az, 'b', 'LineWidth',1); grid(ax4, 'on');
ylabel(ax4, 'Accel (G)'); xlabel(ax4, 'Time (s)');
legend(ax4, 'A_x','A_y','A_z');
title(ax4, 'All Axes'); ylim(ax4, [-16 16]);

%% Figure 2: Gyroscope (3-axis) — docked tabs
figure('Name', [fname ' - Gyroscope (3-axis)'], 'Color', 'w', 'WindowStyle', 'docked');
tg = uitabgroup;

tab1 = uitab(tg, 'Title', 'G_x');
ax1 = axes(tab1);
plot(ax1, t, gx, 'LineWidth',1, 'Marker', '.'); grid(ax1, 'on');
ylabel(ax1, 'G_x (deg/s)');
title(ax1, 'G_x');

tab2 = uitab(tg, 'Title', 'G_y');
ax2 = axes(tab2);
plot(ax2, t, gy, 'LineWidth',1, 'Marker', '.'); grid(ax2, 'on');
ylabel(ax2, 'G_y (deg/s)');
title(ax2, 'G_y');

tab3 = uitab(tg, 'Title', 'G_z');
ax3 = axes(tab3);
plot(ax3, t, gz, 'LineWidth',1, 'Marker', '.'); grid(ax3, 'on');
ylabel(ax3, 'G_z (deg/s)'); xlabel(ax3, 'Time (s)');
title(ax3, 'G_z');

tab4 = uitab(tg, 'Title', 'All Axes');
ax4 = axes(tab4);
plot(ax4, t, gx, 'r', t, gy, 'g', t, gz, 'b', 'LineWidth',1); grid(ax4, 'on');
ylabel(ax4, 'Gyro (deg/s)'); xlabel(ax4, 'Time (s)');
legend(ax4, 'G_x','G_y','G_z');
title(ax4, 'All Axes');

%% Figure 3: Spectrograms (Acceleration)
window = hamming(256);
number_of_points_overlap = 128;
number_of_points_fft = 512;

figure('Name',[fname ' - Acceleration Spectrograms'],'Color','w', 'WindowStyle', 'docked');
tiledlayout(2,2,'Padding','compact','TileSpacing','compact');
nexttile;
spectrogram(ax - mean(ax,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Ax Spectrogram'); colorbar; ylim([0, min(4, fs/2/1000)]); grid on;

nexttile;
spectrogram(ay - mean(ay,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Ay Spectrogram'); colorbar; ylim([0, min(4, fs/2/1000)]); grid on;

nexttile;
spectrogram(az - mean(az,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('Az Spectrogram'); colorbar; ylim([0, min(4, fs/2/1000)]); grid on;

nexttile;
spectrogram(a_mag - mean(a_mag,'omitnan'), window, number_of_points_overlap, number_of_points_fft, fs, 'yaxis');
title('|a| Spectrogram'); colorbar; ylim([0, min(4, fs/2/1000)]); grid on;

colormap hot;

return;

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
