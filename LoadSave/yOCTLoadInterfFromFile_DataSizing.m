function [sizeLambda, sizeX, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions)
%This function extract sizing of the data from the dimensions structure

sizeLambda = length(dimensions.lambda.values);
sizeX = length(dimensions.x.index);
sizeY = length(dimensions.y.index);
if isfield(dimensions,'BScanAvg')
    BScanAvgN = length(dimensions.BScanAvg.index);
else
    BScanAvgN = 1;
end
if isfield(dimensions,'AScanAvg')
    AScanAvgN = length(dimensions.AScanAvg.index);
else
    AScanAvgN = 1;
end   