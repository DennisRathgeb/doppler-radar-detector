%% Parameter

R = linspace(1,100,100); %range
c = 300e6;

tToF = 2.* R ./ c; %time Time of Flight
Bsweep = 260e6; %Given by radar
Tsweep = 3e-3;  %Choosen
s = Bsweep ./ Tsweep;
fbeat = s .* tToF;
dR = c/(2*Bsweep);



%% Plot

plot(R,fbeat)

%%

%%fs/2 @ 50 kHz