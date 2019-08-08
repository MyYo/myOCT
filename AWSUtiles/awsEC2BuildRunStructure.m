function ec2RunStructure = awsEC2BuildRunStructure ...
    (ami,pemFilePath,IAMRole,securityGroup,region)
%This function builds a structure used to initialize EC2 instance
%To initialize EC2 instance we will need to define:
%INPUTS:
%   - AMI (image id, pem name, pem file path) that defines: operational system, storage volume,
%       default installed software etc. 
%       Image id + pem are  used as a 'type' of a user name and
%       password as well. To generate a new id / view existing ids go to: 
%       https://us-west-1.console.aws.amazon.com/ec2/v2/home?region=us-west-1#Images:sort=name
%       EXAMPLE: ami = '09f63235c9cc29c52', pemFilePath='MyPem.pem'
%   - IAM Role - what kind of permissions will the instance generated will
%       have, for example if you would like to have S3 access etc. 
%       To see a full list of roles: 
%       https://console.aws.amazon.com/iam/home?region=us-east-1#/roles
%       To generate a default most accesible role, go to 
%       Create New -> EC2 and tick 'AdministratorAccess' -> Next -> Next
%       EXAMPLE: IAMRole = 'MyRole'
%   - securityGroup - defines who can connect to the instance (ip address
%       etc).
%       To view & defnine security groups:
%       https://console.aws.amazon.com/ec2/v2/home?region=us-east-1#SecurityGroups:sort=groupId
%       To generate a new security group with all access:
%       Go to "Create Security Group" enter name, click on "Inbound" -> "Add Rule" and
%       set: Type: SSH, Source: custom: 0.0.0.0/0, 
%       EXAMPLE: securityGroup = 'mySecurity'
%   - region - what AWS region to use, default: default region init right
%       now
%       EXAMPLE: securityGroup = 'us-west-1'

ec2RunStructure.ami = ami;

if ~exist(pemFilePath,'file')
	error('Cannot find PEM file path at: %s',pemFilePath);
end
[~,tmp] = fileparts(pemFilePath);
ec2RunStructure.pemName = tmp; %By default pem Name is the same as pemFilePath's name
ec2RunStructure.pemFilePath = pemFilePath;

ec2RunStructure.IAMRole = IAMRole;
ec2RunStructure.securityGroup = securityGroup;

if ~exist('region','var') || isempty(region)
    region = awsGetRegion();
end
ec2RunStructure.region = region;