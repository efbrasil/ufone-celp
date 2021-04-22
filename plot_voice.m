% $Id: plot_voice.m 26 2004-05-15 16:09:07Z efb $
%
% plot two files
function plot_voice (size)

	fd1 = fopen ('input.raw', 'rb');
	fd2 = fopen ('output.raw', 'rb');

	x1 = fread (fd1, inf, 'short');
	x2 = fread (fd2, inf, 'short');

	fclose (fd1);
	fclose (fd2);

	subplot (211);
	plot (x1);
	subplot (212);
	plot (x2);
