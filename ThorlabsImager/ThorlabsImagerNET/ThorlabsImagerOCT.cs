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
        [DllImport("ThorlabsImager.dll")]
        public static extern void yOCTScannerInit(string probeFilePath); // Probe ini path.Can be usually found at: C:\\Program Files\\Thorlabs\\SpectralRadar\\Config\\ 

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

        public static void Hi()
        {
            return;
        }
    }
}
