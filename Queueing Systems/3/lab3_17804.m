close all;
clear;
clc;

k=mod(17804,2)+3;
L=2^k;
Nsymb=20000;
nsamp=16;
P_error=zeros(1,2000);
BER=zeros(1,2000);
EbNo=0:0.01:19.99;

%   (i)με τη σχέση (3.33) και τη δεδομένη προσέγγιση
SNR=10.^(EbNo/10);
for i=1:2000
    P_error(i)=((L-1)/L)*erfc(sqrt((3*log2(L)* SNR(i))/(L^2 - 1)));
    BER(i)=P_error(i)/log2(L);
end

figure(1);
semilogy(EbNo,BER);
xlabel('Eb/No (dB)');
ylabel('BER');
grid on;
hold on;

%   (ii)
errors=zeros(1,2000);

for i=1:10
    errors(i*200-100)=ask_errors_2(k,Nsymb,nsamp,EbNo(i*200-100))/(k*Nsymb);
    semilogy(EbNo(i*200-100),errors(i*200-100),'r*')
end
