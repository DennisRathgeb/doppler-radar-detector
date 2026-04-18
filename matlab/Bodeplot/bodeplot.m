%%Q-Teil
data_q = csvread('bryan_beni_Q_Stage.csv');
data_i = csvread('bryan_beni_I_Stage.csv');
data_sim = csvread('MF_bandpass.csv');

data_q(:,1) = 10.^data_q(:,1);
data_i(:,1) = 10.^data_i(:,1);
data_sim(:,1) = 10.^data_sim(:,1);
diff_gain = data_q(:,2) - data_i(:,2);
diff_phase = data_q(:,3) - data_i(:,3);

subplot(2,2,1);
semilogx(data_q(:,1),data_q(:,2),'-b',data_i(:,1),data_i(:,2),'r',data_q(:,1),data_sim(:,2),'m');
xlabel("Frequency");
ylabel("Gain[db]")

subplot(2,2,2);
semilogx(data_q(:,1),diff_gain);
xlabel("Frequency");
ylabel("Gain[db]")

subplot(2,2,3);
semilogx(data_q(:,1),data_q(:,3),'-b',data_i(:,1),data_i(:,3),'r',data_q(:,1),data_sim(:,3),'m');
xlabel("Frequency");
ylabel("Phase")

subplot(2,2,4);
semilogx(data_q(:,1),diff_phase);
xlabel("Frequency");
ylabel("Phase")
