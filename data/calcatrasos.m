clear all;
close all;

testes = [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20];

resultado = "";

for i=1:length(testes),
	clear x d dif ccov clag cmax cind lag d2;
	dirname = sprintf ('./teste%d', testes(i));

	x_fname = sprintf ('%s/adapt_x.wav', dirname);
	d_fname = sprintf ('%s/adapt_d.wav', dirname);
	x = wavread (x_fname);
	d = wavread (d_fname);

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
	[cmax cind] = max (ccov);
	lag = abs (clag (cind)) - 3;
	resultado = sprintf ('%s\n%d', resultado, abs (clag (cind)));
end
