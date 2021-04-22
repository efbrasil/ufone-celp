% FUNCTION FOR CREATING A FIXED CODEBOOK
function create = fc_create(size)
% FC_CREATE - Creates a binary file containing a fixed dicionary which size is determined
%               in the argument. It is for use in the CELP codec.



    n = 40;
    fc_size = (2 * size) - 2 + n;
    codebook = randn(1, fc_size);

	for k = 1:fc_size,
	    if (abs(codebook(k))<1.645),
		codebook(k) = 0;
  	    end;
	end;
        
    name = strcat('codebook', int2str(size), '.dat');
    fid = fopen(name, 'w');
    fwrite(fid, codebook, 'double');
    fs = fclose(fid);
