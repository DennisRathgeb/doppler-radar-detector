##################################################################################
## Projektname:     Calculate the Tolerances
## Filename:        main.py
## Ersteller:       Bryan Uhlmann
## Datum:           02. 04. 2025
## Python-Version:  3.11.4
##################################################################################
import numpy as np

R1 = 1e3
R3 = 10e3

R_Tol_Min = 0.99
R_Tol_Max = 1.01

def gain_LPF(R1, R3):
    return(-R3/R1)

def gain_inv_Amp(R1,R2):
    return(-R2/R1)

K = gain_LPF(R1, R3)
K_dB_normal = 20*np.log10(abs(K))
print(f'Normal gain: {K_dB_normal} dB')

R3 = R3 * R_Tol_Max
R1 = R1 * R_Tol_Min
K = gain_LPF(R1, R3)
K_dB_max = 20*np.log10(abs(K))
print(f'Max gain: {K_dB_max} dB')
print(f'Upper Tolerance: {K_dB_max-K_dB_normal} dB')

R1 = 1e3
R3 = 10e3
R3 = R3 * R_Tol_Min
R1 = R1 * R_Tol_Max
K = gain_LPF(R1, R3,)
K_dB_min = 20*np.log10(abs(K))
print(f'Min gain: {K_dB_min} dB')
print(f'Lower Tolerance: {K_dB_normal-K_dB_min} dB')

## Inventierender Verstärker
R4 = 8.2e3
R5 = 1e3

G = gain_inv_Amp(R5, R4)
G_dB_normal = 20*np.log10(abs(G))
print(f'Normal gain INV: {G_dB_normal} dB')

R4 = R4 * R_Tol_Max
R5 = R5 * R_Tol_Min
G = gain_inv_Amp(R5, R4)
G_dB_max = 20*np.log10(abs(G))
print(f'Max gain INV: {G_dB_max} dB')
print(f'Upper Tolerance INV: {G_dB_max-G_dB_normal} dB')

R4 = 8.2e3
R5 = 1e3
R4 = R4 * R_Tol_Min
R5 = R5 * R_Tol_Max
G = gain_inv_Amp(R5, R4)
G_dB_min = 20*np.log10(abs(G))
print(f'Min gain INV: {G_dB_min} dB')
print(f'Lower Tolerance INV: {G_dB_normal-G_dB_min} dB')

## Total stage
print(f'TOTAL: Normal gain: {K_dB_normal+K_dB_normal+G_dB_normal} dB')
print(f'TOTAL: Max gain: {K_dB_max+K_dB_max+G_dB_max} dB')
print(f'TOTAL: Upper Tolerance: {K_dB_max-K_dB_normal+K_dB_max-K_dB_normal+G_dB_normal-G_dB_min} dB')
print(f'TOTAL: Min gain: {K_dB_min+K_dB_min+G_dB_min} dB')
print(f'TOTAL: Lower Tolerance: {K_dB_normal-K_dB_min+K_dB_normal-K_dB_min+G_dB_normal-G_dB_min} dB')

Input_value = 5.137e-3 # in mV
Output_value = 1.2407 # in mV

Gain = Output_value / Input_value
Gain_dB = 20*np.log10(abs(Gain))
print(Gain_dB)
Test = 20*np.log10(abs(820))
print(Test)
