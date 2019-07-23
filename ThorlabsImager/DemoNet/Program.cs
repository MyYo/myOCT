using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

using ThorlabsImagerNET;

namespace DemoNet
{
    class Program
    {
        static void Main(string[] args)
        {

            #region OCT Scan & Photobleach

            //Init
            Console.WriteLine("yOCTScannerInit");
            ThorlabsImager.yOCTScannerInit(@"C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini");

            Console.WriteLine("yOCTScan3DVolume");
            ThorlabsImager.yOCTScan3DVolume(
                0, 0, 1, 1, //startX, startY, rangeX, rangeY[mm]
                0,          //rotationAngle[deg]
                100, 3,     //SizeX, sizeY[# of pixels]
                2,          //B Scan Average
                @"scan"      //Output directory, make sure it exists before running this function
            );
            Console.WriteLine();
            Console.WriteLine("yOCTScan3DVolume - Done");
            Console.WriteLine();

            Console.WriteLine("yOCTPhotobleachLine");
            ThorlabsImager.yOCTTurnLaser(true);
            ThorlabsImager.yOCTPhotobleachLine( 
                -1, 0, 1, 0, //startX, startY, endX, endY[mm]
                10,           //duration[sec]
                10            //repetitions, how many passes to photobleach(choose 1 or 2)
            );
            ThorlabsImager.yOCTTurnLaser(false);

            Console.WriteLine("yOCTScannerClose");
            ThorlabsImager.yOCTScannerClose();

            #endregion

            #region Move Stage

            // Initialize Stage(All Z positions will be with respect to that initial one)
            Console.WriteLine("yOCTStageInit");
            double zStart = ThorlabsImager.yOCTStageInit('z');

            //Move
            Console.WriteLine("yOCTStageSetZPosition");
            double dz = 1000; //[um]
            ThorlabsImager.yOCTStageSetPosition('z',
                zStart + dz / 1000 //Movement[mm]
                );

            Thread.Sleep(2*1000); //Sleep for x seconds
         
            // Move back(reset)
            ThorlabsImager.yOCTStageSetPosition('z',zStart);
            ThorlabsImager.yOCTStageClose('z');

            #endregion

            Console.WriteLine("Testing Done");
        }
    }
}
