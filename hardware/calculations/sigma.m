%% Parameter sigma

% sigma_Multi = [1 10 20 50]; %in m2
sigma_1 = 1;        % 
sigma_10 = 10;      % 
sigma_20 = 20;      % 
sigma_50 = 50;      % 
sigma_100 = 100;    %
R = linspace(0.5,60,10000);
c = 300e6;          % Speed of light

% sigma_defined = linspace(1,50,50);
%S = 118;
% max_range = 2^12;
% min_bit   = 2^3;
% max_voltage = 3;
% min_voltage = max_voltage / max_range * min_bit;

%% Round-trip loss
S1 = log10(R./0.0167./(sigma_1.^0.25)).* -40;
S10 = log10(R./0.0167./(sigma_10.^0.25)).* -40;
S20 = log10(R./0.0167./(sigma_20.^0.25)).* -40;
S50 = log10(R./0.0167./(sigma_50.^0.25)).* -40;
S100 = log10(R./0.0167./(sigma_100.^0.25)).* -40;
%% Calculations of Frequency
tToF = 2.* R ./ c; %time Time of Flight
Bsweep = 260e6; %Given by radar
Tsweep = 1.0e-3;  %Choosen
s = Bsweep / Tsweep;
fbeat = s .* tToF;
dR = c/(2*Bsweep);

%% Plot in Distance 
figure(1)
subplot(2,1,1);
    semilogx(R,S1,'b',R,S10,'r',R,S20,'y',R,S50,'m',R,S100,'g');
    yline(-117,'-r')
    legend("1m^2","10m^2","20m^2","50m^2","100m^2","limit")
    grid on
    xlabel("Distance in m")
    ylabel("Signal loss in dB")

%% Plot in Distance 
subplot(2,1,2);
    semilogx(fbeat,S1,'b',fbeat,S10,'r',fbeat,S20,'y',fbeat,S50,'m',fbeat,S100,'g');
    yline(-117,'-r')
    legend("1m^2","10m^2","20m^2","50m^2","100m^2","limit")
    grid on
    xlabel("Received frequency in Hz")
    ylabel("Signal loss in dB")
    %%fs/2 @ 50 kHz