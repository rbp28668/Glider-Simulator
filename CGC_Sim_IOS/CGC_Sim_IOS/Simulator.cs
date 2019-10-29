using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using LockheedMartin.Prepar3D.SimConnect;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using Newtonsoft.Json;


//using System.Runtime.InteropServices;

namespace CGC_Sim_IOS
{


    public enum DataRequestID
    {
        Weather_Data = 0,
        Location_Data = 1,
    }

    enum REQUESTS
    {
        REQUEST_4S,
        REQUEST_SIMSTATE,
    }

    enum EVENTS
    {
        SIMSTART,
        FOURSECS,
        SIMSTOP,
        PAUSE,
        PAUSE_TOGGLE, //set pause
        ABORT, //Quit without message
        CLOCK, //Hours of Local Time
        CRASH_RESET,
        SLEW,
        SLEW_FREEZE,
        SLEW_LEFT,
        SLEW_RIGHT,
        SLEW_AHEAD_PLUS,
        SLEW_AHEAD_MINUS,
        AXIS_SLEW_AHEAD_SET,
        SLEW_HEADING_PLUS,
        SLEW_HEADING_MINUS,
        SLEW_ALTIT_PLUS,
        SLEW_ALTIT_MINUS,
        SLEW_ALTIT_UP_SLOW,
        SLEW_ALTIT_DN_SLOW,
        SLEW_ALTIT_UP_FAST,
        SLEW_ALTIT_DN_FAST,
        TOW_PLANE_REQUEST,
        TOW_PLANE_RELEASE,
        SITUATION_RESET,
        SITUATION_SAVE,
        FREEZE_LATITUDE_LONGITUDE_TOGGLE,
    }

    enum GROUP_IDS
    {
        GROUP_0,
        GROUP_1,
    }

    enum MetarElementType
    {
        Station_ID,
        Report_Type,
        Auto,
        Corrected,
        DateTime,
        Nil,
        SurfaceWind,
        SurfaceWindExtension,
        WindsAloft,
        WindsAloftExtension,
        MinMaxWindDir,
        Cavok,
        Visibility,
        RunwayVisualRange,
        PresentConditions,
        PartialObscuration,
        SkyConditions,
        SkyConditionsExtension,
        Temperature,
        TemperatureExtension,
        Altimeter,
        WindsToFollow,
        Unknown,
    }

    class Simulator
    {

        // User-defined win32 event 
        const int WM_USER_SIMCONNECT = 0x0402;
        #region Private Properties

        private Process[] processes;
        private string procName = "Prepar3D";
        private static bool isP3DRunning = false;

        private SimConnect simConnection = null;

        private readonly Metar currentMetar = new Metar();

        #endregion

        #region Public Properties

        /// <summary>
        /// The Logic member
        /// </summary>
        public SimConnect SimConnection
        {
            get
            {
                return simConnection;
            }
        }

        public bool IsP3DRunning
        {
            get
            {
                return isP3DRunning;
            }
        }

        public bool IsPaused { get; set; } = false;

        public Metar CurrentMetar
        {
            get
            {
                return currentMetar;
            }
        }

        #endregion

        public bool OpenSimConnection(IntPtr handle)
        {
            bool rc = false;
            try
            {
                simConnection = new SimConnect("CGC_IOS", handle, WM_USER_SIMCONNECT, null, 0);
                if (simConnection != null)
                {
                    rc = true;
                }
            }
            catch (Exception ex)
            {
                // We were unable to connect so let the user know why. 
                System.Console.WriteLine("Sim Connect unable to connect to Prepar3D!\n\n{0}\n\n{1}",
                                         ex.Message, ex.StackTrace);
            }
            return rc;
        }


        public void CloseSimConnection()
        {
            if (simConnection != null)
            {
                simConnection.Dispose();
                simConnection = null;
            }
        }


        public void StartP3D(string initialScenario)
        {
            if (isP3DRunning == false)
            {
                // Launch P3D
                ProcessStartInfo start = new ProcessStartInfo();
                // Do you want to show a console window?
                start.WindowStyle = ProcessWindowStyle.Hidden;
                start.CreateNoWindow = false;
                start.FileName = "C:\\SimP3D\\Lockheed Martin\\Prepar3D v4\\Prepar3D.exe";

                if (initialScenario == "")
                {
                    Process proc = Process.Start(start);
                }
                else
                {
                    string args = "-fxml: " + initialScenario;
                    Process proc = Process.Start(start.FileName, args, null, null, null);
                }
            }
        }

        public void KillP3D()
        {
            processes = Process.GetProcessesByName(procName);
            foreach (Process proc in processes)
            {
                Process tempProc = Process.GetProcessById(proc.Id);
                tempProc.CloseMainWindow();
                tempProc.WaitForExit();
            }

        }

        public void WeatherRequest()
        {
            if (simConnection != null)
            {
                simConnection.WeatherSetModeGlobal();
                float lat = 0;
                float lng = 0;
                simConnection.WeatherRequestObservationAtNearestStation(DataRequestID.Weather_Data, lat, lng);
            }
        }

        public void WeatherSetWind()
        {
            try
            {
                string newMetar = CurrentMetar.MetarString;
                System.Console.WriteLine("Put Weather Data: " + newMetar);
                simConnection.WeatherSetModeGlobal();
                //simConnection.WeatherSetModeCustom();
                simConnection.WeatherSetObservation(0, newMetar);
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Problem Setting Weather!\n\n{0}\n\n{1}",
                                          ex.Message, ex.StackTrace);
            }
        }

        #region Constructor / Destructor

        public Simulator()
        {
        }

        ~Simulator()
        {
            CloseSimConnection();
        }


        #endregion

    }

    class MetarElement
    {

        public MetarElementType ElementType { get; set; }

        public string ElementTypeStr()
        {
            string str = "???";
            switch (ElementType)
            {
                case (MetarElementType.Station_ID):
                    str = "Station_ID";
                    break;
                case (MetarElementType.Report_Type):
                    str = "Report_Type";
                    break;
                case (MetarElementType.Auto):
                    str = "Auto";
                    break;
                case (MetarElementType.Corrected):
                    str = "Corrected";
                    break;
                case (MetarElementType.DateTime):
                    str = "DateTime";
                    break;
                case (MetarElementType.Nil):
                    str = "Nil";
                    break;
                case (MetarElementType.SurfaceWind):
                    str = "SurfaceWind";
                    break;
                case (MetarElementType.SurfaceWindExtension):
                    str = "SurfaceWindExtension";
                    break;
                case (MetarElementType.WindsAloft):
                    str = "WindsAloft";
                    break;
                case (MetarElementType.WindsAloftExtension):
                    str = "WindsAloftExtension";
                    break;
                case (MetarElementType.MinMaxWindDir):
                    str = "MinMaxWindDir";
                    break;
                case (MetarElementType.Cavok):
                    str = "Cavok";
                    break;
                case (MetarElementType.Visibility):
                    str = "Visibility";
                    break;
                case (MetarElementType.RunwayVisualRange):
                    str = "RunwayVisualRange";
                    break;
                case (MetarElementType.PresentConditions):
                    str = "PresentConditions";
                    break;
                case (MetarElementType.PartialObscuration):
                    str = "PartialObscuration";
                    break;
                case (MetarElementType.SkyConditions):
                    str = "SkyConditions";
                    break;
                case (MetarElementType.SkyConditionsExtension):
                    str = "SkyConditionsExtension";
                    break;
                case (MetarElementType.Temperature):
                    str = "Temperature";
                    break;
                case (MetarElementType.TemperatureExtension):
                    str = "TemperatureExtension";
                    break;
                case (MetarElementType.Altimeter):
                    str = "Altimeter";
                    break;
                case (MetarElementType.WindsToFollow):
                    str = "WindsToFollow";
                    break;
                case (MetarElementType.Unknown):
                    str = "Unknown";
                    break;
            }
            return str;
        }

        protected string subString = "";

        public virtual string SubString
        {
            get
            {
                if (ElementType == MetarElementType.Station_ID)
                    return "GLOB";
                else
                    return subString;
            }
            set
            {
                subString = value;
            }
        }

        public MetarElement(MetarElementType value, string str)
        {
            ElementType = value;
            SubString = str;
            if (ElementType != MetarElementType.SurfaceWind)
                System.Console.WriteLine("MetarElement type : " + ElementTypeStr() + "  " + SubString);

        }
    }

    class MetarWind : MetarElement
    {
        public string DirectionDescription { get; set; } = "";
        public int Speed { get; set; } = 0;
        public int Direction { get; set; } = 0;
        public int Gust { get; set; } = 0;
        public int DepthInMeters { get; set; } = 3000;
        public string Units { get; set; } = "";
        private List<String> extensions = new List<String>();

        public string MetarString
        {
            get
            {
                return "XXXX";
            }
        }

        public MetarWind(MetarElementType value, string str, string units) : base(value, str)
        {
            // parse the str for speed and direction
            Direction = (int)Convert.ToInt32((str.Substring(0, 3)));
            Speed = (int)Convert.ToInt32((str.Substring(3, 2)));
            Units = units;

            int i = 0;
            i = str.IndexOf("G");
            if ((i == 5) || (i == 6))
            {
                // has gusts
                Gust = (int)Convert.ToInt32((str.Substring(i + 1, 2)));
                Console.WriteLine("Gusts {0}", Gust);
            }

            i = str.IndexOf("&");
            if (i >= 0)
            {
                Extension = str.Substring(i);
            }
            System.Console.WriteLine("MetarElement type : " + ElementTypeStr() + "  " + SubString);

        }

        public MetarWind(MetarElementType value, string str, int Speed, int Direction) : base(value, str)
        {

        }

        public override string SubString
        {
            get
            {
                subString = Direction.ToString().PadLeft(3, '0');
                subString += Speed.ToString().PadLeft(2, '0');
                if (Gust != 0)
                {
                    subString += "G" + Gust.ToString().PadLeft(2, '0');
                }
                subString += Units;
                subString += Extension;
                return subString;
            }

            set
            {
                subString = value;
            }
        }

        public string Extension
        {
            get
            {
                string value = "";
                if (extensions.Count == 3)
                {
                    value += extensions[0];
                    if (DepthInMeters > 0)
                    {
                        value += DepthInMeters.ToString();
                    }
                    else
                    {
                        value += extensions[1];
                    }
                    value += extensions[2];
                }
                else
                {
                    foreach (string item in extensions)
                    {
                        value += item;
                    }
                }
                return value;
            }
            set
            {
                int i = value.IndexOf("&");
                if (i >= 0)
                {
                    extensions.Add(value.Substring(0, 2));
                    extensions.Add(value.Substring(2, value.Length - 4));
                    extensions.Add(value.Substring(value.Length - 2));
                }
            }
        }

    }

    class Metar
    {
        private string setMetarString = "";
        private string builtMetarString = "";
        private List<string> currentMetarElements = new List<String>();
        private List<MetarElement> metarElements = new List<MetarElement>();

        public string MetarString
        {
            get
            {
                builtMetarString = "";
                // Build metar string from array of elements
                int surfaceWindElements = 0;
                //bool windsAloft = false;
                foreach (var element in metarElements)
                {
                    // Discard all but the first Surface Wind elements.
                    if (element.ElementType == MetarElementType.SurfaceWind)
                    {
                        if (surfaceWindElements == 0)
                        {
                            builtMetarString += element.SubString + " ";
                            surfaceWindElements++;
                        }
                    }
                    else if (element.ElementType != MetarElementType.WindsAloft &&
                        element.ElementType != MetarElementType.WindsAloftExtension &&
                        element.ElementType != MetarElementType.WindsToFollow)
                    {
                        builtMetarString += element.SubString + " ";
                    }
                }
                builtMetarString.Trim();
                return builtMetarString;
            }
            set
            {
                setMetarString = value;
                // split into subsections

                currentMetarElements.Clear();
                char[] delimiterChars = { ' ' };
                string[] words = setMetarString.Split(delimiterChars);
                foreach (var entry in words)
                {
                    currentMetarElements.Add(entry);
                    Console.WriteLine(entry);
                }
                ParseCurrentMetarElements();
            }
        }

        public int SurfaceWindSpeed
        {
            get
            {
                if (metarElements.Count > 0)
                {
                    foreach (var item in metarElements)
                    {
                        if (item.ElementType == MetarElementType.SurfaceWind)
                        {
                            MetarWind mwItem = (MetarWind)item;
                            return mwItem.Speed;
                        }
                    }
                }
                return 0;
            }
            set
            {
                if (metarElements.Count > 0)
                {
                    foreach (var item in metarElements)
                    {
                        if (item.ElementType == MetarElementType.SurfaceWind)
                        {
                            MetarWind mwItem = (MetarWind)item;
                            mwItem.Speed = value;
                        }
                    }
                }
            }
        }

        public int SurfaceWindDirection
        {
            get
            {
                if (metarElements.Count > 0)
                {
                    foreach (var item in metarElements)
                    {
                        if (item.ElementType == MetarElementType.SurfaceWind)
                        {
                            MetarWind mwItem = (MetarWind)item;
                            return mwItem.Direction;
                        }
                    }
                }
                return 0;
            }
            set
            {
                if (metarElements.Count > 0)
                {
                    foreach (var item in metarElements)
                    {
                        if (item.ElementType == MetarElementType.SurfaceWind)
                        {
                            MetarWind mwItem = (MetarWind)item;
                            mwItem.Direction = value;
                        }
                    }
                }
            }
        }

        private bool IsWindSpeedEntry(string entry, out string units)
        {
            bool bRc = false;
            units = "";
            // check for units
            if ((entry.Contains("KT") || entry.Contains("MPS") || entry.Contains("KMH")))
            {
                int i = 0;
                i = entry.IndexOf("KT");
                if ((i == 4) || (i == 5) || (i == 8) || (i == 9))
                {
                    units = "KT";
                    bRc = true;
                }
                else
                {
                    i = entry.IndexOf("MPS");
                    if ((i == 4) || (i == 5) || (i == 8) || (i == 9))
                    {
                        units = "MPS";
                        bRc = true;
                    }
                    else
                    {
                        i = entry.IndexOf("KMH");
                        if ((i == 4) || (i == 5) || (i == 8) || (i == 9))
                        {
                            units = "KMH";
                            bRc = true;
                        }
                    }
                }
            }
            return bRc;
        }

        private void ParseCurrentMetarElements()
        {
            metarElements.Clear();
            int numElements = currentMetarElements.Count;
            bool bWindsToFollow = false;

            if (numElements > 6)
            {
                int index = 0;
                foreach (var entry in currentMetarElements)
                {
                    if (index == 0)
                    {
                        MetarElement item = new MetarElement(MetarElementType.Station_ID, entry);
                        metarElements.Add(item);
                        index++;
                    }
                    else
                    {
                        if (entry.Contains("METAR") || entry.Contains("SPECI"))
                        {
                            MetarElement item = new MetarElement(MetarElementType.Report_Type, entry);
                            metarElements.Add(item);
                            index++;
                        }
                        else
                        {
                            if (entry.Contains("AUTO"))
                            {
                                MetarElement item = new MetarElement(MetarElementType.Auto, entry);
                                metarElements.Add(item);
                                index++;
                            }
                            else
                            {
                                if (entry.Contains("COR"))
                                {
                                    MetarElement item = new MetarElement(MetarElementType.Corrected, entry);
                                    metarElements.Add(item);
                                    index++;
                                }
                                else
                                {
                                    if (entry.Contains("NIL"))
                                    {
                                        MetarElement item = new MetarElement(MetarElementType.Nil, entry);
                                        metarElements.Add(item);
                                        index++;
                                    }
                                    else
                                    {
                                        // test for windspeed
                                        string units = "";
                                        if (IsWindSpeedEntry(entry, out units))
                                        {
                                            MetarElement item = new MetarWind(MetarElementType.SurfaceWind, entry, units);
                                            metarElements.Add(item);
                                            index++;
                                        }
                                        else
                                        {
                                            if ((entry.Length == 7) && (entry.Substring(3, 1) == "V"))
                                            {
                                                MetarElement item = new MetarElement(MetarElementType.MinMaxWindDir, entry);
                                                metarElements.Add(item);
                                                index++;
                                            }
                                            else
                                            {
                                                if (entry.Contains("CAVOK"))
                                                {
                                                    MetarElement item = new MetarElement(MetarElementType.Cavok, entry);
                                                    metarElements.Add(item);
                                                    index++;
                                                }
                                                else
                                                {
                                                    if ((entry.Length == 6) && (entry.Substring(0, 1) == "A") || ((entry.Length == 6) && (entry.Substring(0, 1) == "Q")))
                                                    {
                                                        MetarElement item = new MetarElement(MetarElementType.Altimeter, entry);
                                                        metarElements.Add(item);
                                                        index++;
                                                    }
                                                    else
                                                    {
                                                        if (entry == "@@@")
                                                        {
                                                            MetarElement item = new MetarElement(MetarElementType.WindsToFollow, entry);
                                                            metarElements.Add(item);
                                                            bWindsToFollow = true;
                                                            index++;
                                                        }
                                                        else
                                                        {
                                                            if (bWindsToFollow == false)
                                                            {
                                                                MetarElement item = new MetarElement(MetarElementType.Unknown, entry);
                                                                metarElements.Add(item);
                                                                index++;
                                                            }
                                                            else
                                                            {
                                                                MetarElement item = new MetarElement(MetarElementType.WindsAloft, entry);
                                                                metarElements.Add(item);
                                                                index++;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //                    System.Console.WriteLine("Index {0} ", index);
                }
            }
        }
    }


    class SimRestConnection
    {
        HttpClient client = new HttpClient();
        private bool failedAltimeter = false;
        private bool failedASI = false;
        private bool failedPitot = false;
        private bool failedElectrics = false;
        private bool failedTurnCord = false;

        #region Public Properties

        public bool AltimeterFailed
        {
            get
            {
                return failedAltimeter;
            }

            set
            {

            }
        }
        public bool ASIFailed
        {
            get
            {
                return failedASI;
            }

            set
            {

            }
        }

        public bool ElectricsFailed
        {
            get
            {
                return failedElectrics;
            }

            set
            {

            }
        }

        public bool PitotFailed
        {
            get
            {
                return failedPitot;
            }

            set
            {

            }
        }

        #endregion

        public SimRestConnection()
        {
            // Update port # in the following line. 
            //client.BaseAddress = new Uri("http://localhost/p3dapi/"); 
            //client.DefaultRequestHeaders.Accept.Clear(); 
            //client.DefaultRequestHeaders.Accept.Add( 
            //    new MediaTypeWithQualityHeaderValue("application/json")); 
        }

        ~SimRestConnection()
        {

        }

        public async Task<dynamic> RunCmdAsync(string[] args)
        {
            string responseBody = "";
            dynamic response = null; ;
            if (args.Length < 2)
            {
                Console.WriteLine("Need to supply at least the command and its sub-command");
                return response;
            }

            // Call asynchronous network methods in a try/catch block to handle exceptions. 
            try
            {
                // This just builds up the full URL from bits on the command line as a convenience. 
                string url = "http://localhost/p3dapi/";
                int index = 0;
                url += args[index++];
                url += "/";
                url += args[index++];

                if (args.Length > 2)
                {
                    url += "?";
                    url += WebUtility.HtmlEncode(args[index++]);
                }

                while (index < args.Length)
                {
                    url += "&";
                    //url += WebUtility.UrlEncode(args[index++]);
                    url += WebUtility.HtmlEncode(args[index++]);
                }
                // replace spaces by %20
                url = url.Replace(" ", "%20");
                // Should have a complete URL with all the parameters 
                Console.WriteLine(url);


                responseBody = await Run(url);//.Result; 

                Console.WriteLine(responseBody);

                response = parseResult(responseBody);

            }
            catch (HttpRequestException e)
            {
                Console.WriteLine("\nException Caught!");
                Console.WriteLine("Message :{0} ", e.Message);
            }
            return response;
        }

        dynamic parseResult(string json)
        {
            dynamic data = JsonConvert.DeserializeObject(json);
            bool isOK = data.status == "OK";
            if (isOK)
            {
                // do something creative here.... 
            }
            return data;
            
        }

        // This is the key bit that actually calls the web API - assuming it's 
        // given a properly formed url that matches the web api. 
        // Note async nature. 
        async Task<string> Run(string url)
        {
            HttpResponseMessage response = await client.GetAsync(url);
            response.EnsureSuccessStatusCode();
            string responseBody = await response.Content.ReadAsStringAsync();
            // Above three lines can be replaced with new helper method below 
            // string responseBody = await client.GetStringAsync(uri); 
            return responseBody;
        }

        public void CMD_Pause()
        {

            string[] cmd = { "cmd", "pause_toggle"};
            dynamic response = RunCmdAsync(cmd);
            ////RunAsync().GetAwaiter().GetResult(); 
            //string[] cmd2 = { "scenario", "list" }; 
            //RunCmdAsync(cmd2); 
            //string[] cmd3 = { "position", "up?feet=2000" }; 
            //RunCmdAsync(cmd3); 
            //string[] cmd3 = { "traffic", "launch" };
            //RunCmdAsync(cmd3);
        }

        public void CMD_Freeze_Lat_Long()
        {
            string[] cmd = { "cmd", "FREEZE_LATITUDE_LONGITUDE_SET" };
            RunCmdAsync(cmd);
        }

        public void CMD_Position_Back(int count=10)
        {
            string[] cmd = { "position", "back?count="+count.ToString() };
            RunCmdAsync(cmd);
        }

        public void CMD_Position_Set(int count = 1)
        {
            string[] cmd = { "position", "set?count=" + count.ToString() };
            RunCmdAsync(cmd);
        }

        public async Task<int> CMD_Position_Is_OnGround()
        {
            int onGround = 0;
            try
            {
                string[] cmd = { "position", "current" };
                dynamic response = await RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    onGround = (int)jRep["current"]["on_ground"];
                }
            }
            catch (Exception ex)
            {
            }
            return onGround;
        }


        public async Task<int> CMD_Position_Available()
        {
            int length = 0;
            try
            {
                string[] cmd = { "position", "available" };
                dynamic response = await RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    length = (int)jRep["length"];
                }
            }
            catch (Exception ex)
            {
            }
            return length;
        }

        public void CMD_Position_Clear_History()
        {
            int length = 0;
            try
            {
                string[] cmd = { "position", "clear" };
                RunCmdAsync(cmd);
            }
            catch (Exception ex)
            {
            }
        }

        public async Task<bool> Get_Failure_States()
        {
            bool changed = false;
            try
            {
                string[] cmd = { "failures", "current" };
                dynamic response = await RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    failedAltimeter = (bool)jRep["altimeter"];
                    failedASI = (bool)jRep["airspeed"];
                    failedPitot = (bool)jRep["pitot"];
                    failedElectrics = (bool)jRep["electrical"];
                    failedTurnCord = (bool)jRep["turn_coordinator"];
                    changed = true;
                }
            }
            catch (Exception ex)
            {
            }
            return changed;
        }

    }
}