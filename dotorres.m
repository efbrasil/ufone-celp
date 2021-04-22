% $Id: dotorres.m 59 2004-06-06 22:54:16Z efb $
%
% Pega os arquivos ./torres/mat/*.mat, junta com os dados
% de ./torres/dat/????/*.dat, e salva no
% ./torres/results/????.mat
%
function ret = dotorres (a)

aux = dir ('./torres/mat');
num_files = length (aux);
mat_files = aux (3:num_files);
num_files = num_files - 2;

tot = 0;

for i=1:num_files,
	tot = tot + 1;

	base_name = mat_files(i).name(1:4);
	fprintf ('Tratando o difone %s\n', base_name);

	orig_mat_file = sprintf ('./torres/mat/%s', mat_files(i).name);
	load (orig_mat_file);

	arqLSF = sprintf ('./torres/dat/%s/lsf.dat', base_name);
	arqLSF_ind = sprintf ('./torres/dat/%s/lsf_index.dat', base_name);
	arqAGain_ind = sprintf ('./torres/dat/%s/again_ind.dat', base_name);
	arqFGain_ind = sprintf ('./torres/dat/%s/fgain_ind.dat', base_name);
	arqAGain = sprintf ('./torres/dat/%s/again.dat', base_name);
	arqAInd = sprintf ('./torres/dat/%s/aind.dat', base_name);
	arqFGain = sprintf ('./torres/dat/%s/fgain.dat', base_name);
	arqFInd = sprintf ('./torres/dat/%s/find.dat', base_name);

	fileLSF     = fopen(arqLSF, 'rb');
	fileLSF_ind = fopen(arqLSF_ind, 'rb');
	fileAGain_ind = fopen(arqAGain_ind, 'rb');
	fileFGain_ind = fopen(arqFGain_ind, 'rb');
	fileAg      = fopen(arqAGain, 'rb');
	fileFg      = fopen(arqFGain, 'rb');
	fileAi      = fopen(arqAInd, 'rb');
	fileFi      = fopen(arqFInd, 'rb');

	if ( (fileLSF == -1) | (fileAg == -1) | (fileFg == -1) | (fileAi == -1) | (fileFi == -1) ),
		error('Nao abriu os arquivos de saida do codificador!');
	end

	%Lendo os resultads da codificacao.
	sLSF       = fread(fileLSF, inf, 'double')';
	AGain_ind = fread(fileAGain_ind, inf, 'int')';
	FGain_ind = fread(fileFGain_ind, inf, 'int')';
	sLSF_ind   = fread(fileLSF_ind, inf, 'int')';
	iFixo      = fread(fileFi, inf, 'int')';
	gFixo      = fread(fileFg, inf, 'double')';
	iAdap      = fread(fileAi, inf, 'int')';
	gAdap      = fread(fileAg, inf, 'double')';

	fclose(fileAGain_ind);
	fclose(fileFGain_ind);
	fclose(fileLSF_ind);
	fclose(fileLSF);
	fclose(fileAg);
	fclose(fileFg);
	fclose(fileAi);
	fclose(fileFi);

	result_file_name = sprintf ('./torres/results/%s.mat', base_name);
	eval (sprintf ('save %s sLSF sLSF_ind AGain_ind FGain_ind iFixo gFixo iAdap gAdap trm vzmt meiodif', result_file_name));

end

fprintf('Total de %d arquivos processados.\n', tot)
