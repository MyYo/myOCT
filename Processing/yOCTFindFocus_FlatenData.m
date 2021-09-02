function data = yOCTFindFocus_FlatenData(scan,scanAbs,dim)

data = mean(scanAbs,[2,3,4]); % X,Y, A or B Scan Averaging