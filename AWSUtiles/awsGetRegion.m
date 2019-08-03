function region = awsGetRegion()
%This function gets the active region used by AWS as set by the environment
%varible, (level = 0)

region = getenv('AWS_REGION');