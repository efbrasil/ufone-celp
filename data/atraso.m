clear all;
close all;

x = wavread ('adapt_x.wav');
d = wavread ('adapt_d.wav');

if length (x) > length (d),
	disp "x é maior que d"
	dif = length (x) - length (d);
	d = [d' zeros(1,dif)];
else,
	disp "d é maior que x"
	dif = length (d) - length (x);
	x = [x' zeros(1,dif)];
end

[ccov clag] = xcov (x, d);

[cmax cind] = max (ccov)

lag = abs (clag (cind)) - 3;

d2 = d(lag:end);
wavwrite (d2, 'adapt_d2.wav');

