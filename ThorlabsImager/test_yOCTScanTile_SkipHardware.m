classdef test_yOCTScanTile_SkipHardware < matlab.unittest.TestCase
    % Test yOCTScanTile but while skipping hardware (test just the logic
    % part)
    
    methods(TestClassSetup)
        % Shared setup for the entire test class
    end
    
    methods(TestMethodSetup)
        % Setup for each test
    end
    
    methods(Test)
        function testDefaultParameters3D(testCase)
            json = yOCTScanTile('test', ...
                [-0.25 0.25], ...
                [-0.25 0.25], ...
                'octProbePath', yOCTGetProbeIniPath('40x','OCTP900'),...
                'skipHardware', true);
        end

        function testDefaultParameters3DSetSmallerFOV(testCase)
            json = yOCTScanTile('test', ...
                [-0.25 0.25], ...
                [-0.25 0.25], ...
                'octProbePath', yOCTGetProbeIniPath('40x','OCTP900'),...
                'octProbeFOV_mm', 0.01,...
                'skipHardware', true);
            if length(json.xCenters_mm) < 10
                testCase.verifyFail('Expected to have a lot more centers')
            end

        end

        function testDefaultParameters2DSkipHardware(testCase)
            json = yOCTScanTile('test', ...
                [-0.25 0.25], 0, ...
                'octProbePath', yOCTGetProbeIniPath('40x','OCTP900'),...
                'octProbeFOV_mm', 0.01,...
                'skipHardware', true);
        end
    end
    
end