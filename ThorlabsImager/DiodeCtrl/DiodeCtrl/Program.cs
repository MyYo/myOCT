using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Thorlabs.TL4000; // Laser diode

namespace DiodeCtrl
{
    class Program
    {
        static void Main(string[] args)
        {

            #region Input processing
            if (args.Length == 0)
            {
                Console.WriteLine("How to run:");
                Console.WriteLine("DiodeCtrl.exe on - to turn diode on");
                Console.WriteLine("DiodeCtrl.exe off - to turn diode off");
                return;
            }

            bool onoff = true;
            switch (args[0].ToLower())
            {
                case "on":
                    onoff = true;
                    break;

                case "off":
                    onoff = false;
                    break;

                default:
                    Console.WriteLine("Unknown input, please run DiodeCtrl.exe for input list");
                    return;
            }

            #endregion

            // Implementation code below according to the programing reference (C#)
            // https://www.thorlabs.com/software_pages/ViewSoftwarePage.cfm?Code=4000_Series&viewtab=3

            // Connect to laser diode driver
            string diodeDriverID = "M00511660"; // This is also refered to as S / N when you boot up the driver.
            TL4000 itc = new TL4000("USB0::0x1313::0x804F::" + diodeDriverID + "::0::INSTR", true, false);

            if (onoff)
            {
                // In case user is about to turn diode on, set current limit.
                itc.setLdCurrSetpoint(0.140); // Amps
            }

            // Turn TEC
            itc.switchTecOutput(onoff);

            // Turn LD (if not locked)
            bool isLocked;
            itc.isTrippedLdOutputProtKeylock(out isLocked);
            if (isLocked)
            {
                Console.WriteLine("Keylock engaged, skipping switching LD.");
                return;
            }
            itc.switchLdOutput(onoff);

            // Get current LD state
            bool currentState;
            itc.getLdOutputState(out currentState);

            // Convert state to string
            string currentStateStr, onoffStr;
            if (currentState)
                currentStateStr = "On";
            else
                currentStateStr = "Off";
            if (onoff)
                onoffStr = "On";
            else
                onoffStr = "Off";


            if (currentState != onoff)
            {
                Console.WriteLine(
                    "Tried to change laser diode state to: " + onoffStr +
                    ", However, current sate is: " + currentStateStr + ". is the keylock engaged? You might want to disingage that."
                    );
            }
        }
    }
}
