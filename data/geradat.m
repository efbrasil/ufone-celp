clear all;
close all;

ruido = wavread ('adapt_x.wav');
cont = wavread ('adapt_d2.wav');

fd = fopen ('ruido.dat', 'wb');
fwrite (fd, ruido, 'double');
fclose (fd);

fd = fopen ('cont.dat', 'wb');
fwrite (fd, cont, 'double');
fclose (fd);
