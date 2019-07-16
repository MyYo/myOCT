using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;     // DLL support

namespace ThorlabsImagerNET
{
    public static class ThorlabsImager
    {

        #region OCT Scanner
        //Initialize OCT Scanner
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTScannerInit(string probeFilePath); // Probe ini path.Can be usually found at: C:\\Program Files\\Thorlabs\\SpectralRadar\\Config\\ 

        //Close OCT Scanner, Cleanup
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTScannerClose();
        
        // Scan a 3D Volume
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTScan3DVolume(
            double xStart, //Scan start position [mm]
            double yStart, //Scan start position [mm]
            double rangeX, //fast direction length [mm]
            double rangeY, //slow direction length [mm]
            double rotationAngle, //Scan angle [deg]
            int sizeX,  //Number of pixels on the fast direction
            int sizeY,  //Number of pixels on the slow direction
            int nBScanAvg, //Number of B scan averages (set to 1 if non)
            string outputDirectory //Output folder, make sure it exists and empty before running this function
            );

        #endregion

        // Photobleach a Line
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTPhotobleachLine(
            double xStart,	//Start position [mm]
            double yStart,	//Start position [mm]
            double xEnd,		//End position [mm]
            double yEnd,		//End position [mm]
            double duration,	//How many seconds to photobleach
            double repetition //How many times galvoes should go over the line to photobleach. slower is better. recomendation: 1
            );

        //Initialize Stage
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTStageInit();

        //Get / Set Stage Position [mm]
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTStageSetZPosition(double newZ);

        public static void Hi()
        {
            return;
        }
    }
}
