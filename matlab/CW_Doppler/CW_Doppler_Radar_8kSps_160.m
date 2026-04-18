% ============================================================
% PM4, CW-Doppler-Radar, Exercise 3
% ============================================================
clear; close all; clc;

% Input
% ============================================================
[y,fs] = audioread('cw_doppler_radar_8kSps_160.wav');
y = y'; % y(1,:) corresponds to I(t), y(2,:) to Q(t)

% Parameter
% ============================================================
N = length(y(1,:));     % Total number of samples
NFFT = 256;             % FFT block size

% Time vector
t = (0:N-1) / fs * 1000; % Convert to milliseconds

% Plot sensor signals
% ============================================================
subplot(3,1,1);
plot(t,y(1,:),t,y(2,:),'--','LineWidth',1.0); 
grid on;
axis([0 40 -1.1 1.1])
xlabel('t / ms'); 
legend('I(t)','Q(t)');
title('Sensor Signals');

% Doppler frequency detection, single sensor signal
% ============================================================
z = y(1,1:NFFT);         % Take first NFFT samples from I(t)
Z = abs(fft(z)) / NFFT;  % Compute FFT magnitude
Z = fftshift(Z);         % Shift zero frequency to center
f = (-NFFT/2:NFFT/2-1) * fs / NFFT;  % Frequency axis

subplot(3,1,2);
stem(f,Z,'LineWidth',1.0); 
grid on;
axis([-1000 1000 0 max(Z)]);
xlabel('f / Hz'); 
ylabel('abs(Z(f))');
title('Doppler Spectrum for I(t)');

% Doppler frequency detection, using I & Q signals
% ============================================================
z = y(1,1:NFFT) + 1j * y(2,1:NFFT);  % Create complex signal z = I + jQ
Z = abs(fft(z)) / NFFT;              % Compute FFT magnitude
Z = fftshift(Z);                     % Shift zero frequency to center

subplot(3,1,3);
stem(f,Z,'LineWidth',1.0); 
grid on;
axis([-1000 1000 0 max(Z)]);
xlabel('f / Hz'); 
ylabel('abs(Z(f))');
title('Doppler Spectrum for I(t) + jQ(t)');
