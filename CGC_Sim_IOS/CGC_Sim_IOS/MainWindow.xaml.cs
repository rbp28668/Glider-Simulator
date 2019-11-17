using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Forms;

using LockheedMartin.Prepar3D.SimConnect;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections;
using System.IO;
using System.Windows.Interop;
using System.Threading;
using System.Net.Http;
using System.Net.Http.Headers;

namespace CGC_Sim_IOS
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {


        [DllImport("user32.dll")]
        static extern bool SetForegroundWindow(IntPtr hWnd);
        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
        [DllImport("user32.dll", SetLastError = true)]
        static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        public enum GWL
        {
            GWL_WNDPROC = (-4),
            GWL_HINSTANCE = (-6),
            GWL_HWNDPARENT = (-8),
            GWL_STYLE = (-16),
            GWL_EXSTYLE = (-20),
            GWL_USERDATA = (-21),
            GWL_ID = (-12)
        }

        const int WS_MINIMIZE = 0x20000000;
        private Simulator sim = new Simulator();
        private SimRestConnection simRest = new SimRestConnection();
        private int rewindCount = 1;
        private bool rewindActive = false;

        private bool statusOnGround = true;
        private bool statusAerotowRequested = false;
        private bool statusWinchLaunchRequested = false;
        private int statusAerotowAbandon = 0;
        private uint statusLatLongFrozen = 1;

        private float currentLatitude = 0.0f;
        private float currentLongitude = 0.0f;

        private Thread closeWinchXDialog = null;
        private Thread minimiseP3DControl = null;
        private Thread minimiseP3DInstruments1 = null;
        private Thread minimiseP3DToPDA = null;

        private List<Scenario> Scenarios = new List<Scenario>();
        private List<String> Scenario_Airfields = new List<String>();
        private List<Scenario> Scenarios_For_Airfield = new List<Scenario>();
        private string defaultScenario = "";
        private string defaultAirfield = "";

        private List<TrafficSpeed> trafficSpeeds = new List<TrafficSpeed>();
        private List<TrafficRange> trafficRanges = new List<TrafficRange>();
        private List<TrafficHeight> trafficHeights = new List<TrafficHeight>();
        private List<TrafficHeading> trafficHeadings = new List<TrafficHeading>();
        private List<WeatherTheme> weatherThemes = new List<WeatherTheme>();

        



        private Scenario selectedScenario = null;
        private bool p3DAlreadyRunning = false;

        private bool simPausedOnStartingSlew = false;
        private bool simSlewing = false;

        private List<String> Traffic_Aircraft = new List<String>();
        private string traffic_Aircraft = "";


        const uint SIMCONNECT_OBJECT_ID_USER = 0;           // proxy value for User vehicle ObjectID
        const uint DATA = 0;

        const int SW_SHOWMINNOACTIVE = 7;

        public SimConnect SimConnection
        {
            get
            {
                if (sim.SimConnection == null)
                {
                    if (!p3DAlreadyRunning)
                    {
                        if (sim.OpenSimConnection(handle))
                        {
                            InitSimConnectionEvents();
                            p3DAlreadyRunning = true;
                        }
                    }
                }
                return sim.SimConnection;
            }
        }

        private IntPtr handle;
        private HwndSource handleSource;

        // User-defined win32 event
        const int WM_USER_SIMCONNECT = 0x0402;
        TextBoxOutputter outputter;

        // Register WPF window
        public MainWindow()
        {
            InitializeComponent();
            outputter = new TextBoxOutputter(TestBox);
            trafficSpeeds.Add(new TrafficSpeed(50));
            trafficSpeeds.Add(new TrafficSpeed(75));
            trafficSpeeds.Add(new TrafficSpeed(100));
            trafficSpeeds.Add(new TrafficSpeed(150));
            trafficSpeeds.Add(new TrafficSpeed(200));
            trafficSpeeds.Add(new TrafficSpeed(300));
            trafficSpeeds.Add(new TrafficSpeed(400));

            trafficRanges.Add(new TrafficRange(0.25f));
            trafficRanges.Add(new TrafficRange(0.5f));
            trafficRanges.Add(new TrafficRange(0.75f));
            for (float range=1; range <=10; range++)
            {
                trafficRanges.Add(new TrafficRange(range));
            }

            trafficHeights.Add(new TrafficHeight(1000));
            trafficHeights.Add(new TrafficHeight(500));
            trafficHeights.Add(new TrafficHeight(250));
            trafficHeights.Add(new TrafficHeight(100));
            trafficHeights.Add(new TrafficHeight(50));
            trafficHeights.Add(new TrafficHeight(0));
            trafficHeights.Add(new TrafficHeight(-50));
            trafficHeights.Add(new TrafficHeight(-100));
            trafficHeights.Add(new TrafficHeight(-250));
            trafficHeights.Add(new TrafficHeight(-500));
            trafficHeights.Add(new TrafficHeight(-1000));

            for (int heading = -180; heading <= 180; heading += 10)
            {
                trafficHeadings.Add(new TrafficHeading(heading));
            }
            

#if (DEBUG)
#else
            Console.SetOut(outputter);
#endif

        }

        ~MainWindow()
        {
#if (DEBUG)
#else
            CloseSimConnection();
            sim.KillP3D();
            p3DAlreadyRunning = false;
#endif

            if (handleSource != null)
            {
                handleSource.RemoveHook(HandleSimConnectEvents);
            }
        }
       
        #region CGCWindows

        private void closeWinchXDialogTask(object obj)
        {
            bool closed = false;
            int loopCount = 0;
            int time = 5000;
            do
            {
                Process[] p = Process.GetProcessesByName("WinchX");
                if (p.Count() > 0)
                {
                    // check that it's not the main WinchX! dialog showing
                    if (!p[0].MainWindowTitle.Contains("!"))
                    {
                        SetForegroundWindow(p[0].MainWindowHandle);
                        SendKeys.SendWait("N");
                        Console.WriteLine("WinchX dialog closed");
                    }
                    closed = true;
                    time = 15000;
                }
                Thread.Sleep(time);
                loopCount++;
            } while (true);
        }

        private void minimiseTask(object obj)
        {
            string processName = (string)obj;
            int loop = 0;
            int numToMinimise = 1;
            if (processName == "P3DInstruments")
            {
                numToMinimise++;
            }
            while (numToMinimise > 0 && loop < 10)
            {
                Process[] pArray = Process.GetProcessesByName(processName);
                foreach(Process p in pArray)
                {
                    var handle = p.MainWindowHandle;
                    // Check if Window is already
                    int style = GetWindowLong(p.MainWindowHandle, (int)GWL.GWL_STYLE);
                    if (!((style & WS_MINIMIZE) == WS_MINIMIZE))
                    {
                        //It's not minimized
                        ShowWindow(handle, SW_SHOWMINNOACTIVE);
                        Console.WriteLine(processName + " minimised");
                        numToMinimise--;
                    }
                 }
                Thread.Sleep(5000);
                loop++;
            }
            Console.WriteLine(processName + " exiting");
        }

        #endregion

        private void LaunchP3D()
        {
            Process[] p = Process.GetProcessesByName("Prepar3D");
            if (p.Count() == 0)
            {
                sim.StartP3D(defaultScenario);
            }

            if (!p3DAlreadyRunning)
            {
                //p3DAlreadyRunning = true;
                closeWinchXDialog = new Thread(closeWinchXDialogTask);
                closeWinchXDialog.Start();

                minimiseP3DControl = new Thread(minimiseTask);
                minimiseP3DControl.Start("P3DControl");

                minimiseP3DInstruments1 = new Thread(minimiseTask);
                minimiseP3DInstruments1.Start("P3DInstruments");

                minimiseP3DToPDA = new Thread(minimiseTask);
                minimiseP3DToPDA.Start("P3DToPDA");

                SimConnect temp = SimConnection;
                // Toggling Pause twice is a bodge to update the Pause button without changing the state of Pause.                Thread.Sleep(5000);
                //simRest.CMD_Pause();
                //Thread.Sleep(1000);
                //simRest.CMD_Pause();
                UpdateFailureButtons();

 
            }
        }


        #region P3D Events

        private IntPtr HandleSimConnectEvents(IntPtr hWnd, int message, IntPtr wParam, IntPtr lParam, ref bool isHandled)
        {
            isHandled = false;

            switch (message)
            {
                case WM_USER_SIMCONNECT:
                    {
                        if (SimConnection != null)
                        {
                            SimConnection.ReceiveMessage();
                            isHandled = true;
                        }
                    }
                    break;

                default:
                    break;
            }

            return IntPtr.Zero;
        }


        // Set up all the SimConnect related event handlers 
        private void InitSimConnectionEvents()
        {
            try
            {
                if (SimConnection != null)
                {
                    // listen to connect and quit msgs 
                    SimConnection.SetNotificationGroupPriority(GROUP_IDS.GROUP_1, SimConnect.SIMCONNECT_GROUP_PRIORITY_HIGHEST);
                    SimConnection.OnRecvOpen += new SimConnect.RecvOpenEventHandler(SimCon_OnRecvOpen);
                    SimConnection.OnRecvQuit += new SimConnect.RecvQuitEventHandler(SimCon_OnRecvQuit);

                    // listen to exceptions 
                    SimConnection.OnRecvException += new SimConnect.RecvExceptionEventHandler(SimCon_OnRecvException);

                    // listen to events 
                    SimConnection.OnRecvEvent += new SimConnect.RecvEventEventHandler(SimCon_OnRecvEvent);

                    SimConnection.OnRecvWeatherObservation += new SimConnect.RecvWeatherObservationEventHandler(SimCon_OnRevWeatherObservation);

                    // catch a simobject data request 
                    // Subscribe to system events 
                    SimConnection.SubscribeToSystemEvent(EVENTS.FOURSECS, "4sec");
                    SimConnection.SubscribeToSystemEvent(EVENTS.SIMSTART, "SimStart");
                    SimConnection.SubscribeToSystemEvent(EVENTS.SIMSTOP, "SimStop");
                    SimConnection.SubscribeToSystemEvent(EVENTS.PAUSE, "Pause");
                    SimConnection.SubscribeToSystemEvent(EVENTS.CRASH_RESET, "CrashReset");
                    
                    // Initially turn the events off 
                    SimConnection.SetSystemEventState(EVENTS.FOURSECS, SIMCONNECT_STATE.ON);
                    SimConnection.SetSystemEventState(EVENTS.SIMSTART, SIMCONNECT_STATE.ON);
                    SimConnection.SetSystemEventState(EVENTS.SIMSTOP, SIMCONNECT_STATE.ON);

                    SimConnection.OnRecvSimobjectDataBytype += new SimConnect.RecvSimobjectDataBytypeEventHandler(SimCon_OnRecvSimobjectDataBytype);
                    SimConnection.MapClientEventToSimEvent(EVENTS.PAUSE_TOGGLE, "PAUSE_TOGGLE");

                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW, "SLEW_TOGGLE");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.SLEW, false);

                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_FREEZE, "SLEW_FREEZE");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.SLEW_FREEZE, false);

                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_LEFT, "SLEW_LEFT");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_RIGHT, "SLEW_RIGHT");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_AHEAD_PLUS, "SLEW_AHEAD_PLUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_AHEAD_MINUS, "SLEW_AHEAD_MINUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.AXIS_SLEW_AHEAD_SET, "AXIS_SLEW_AHEAD_SET");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_HEADING_PLUS, "SLEW_HEADING_PLUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_HEADING_MINUS, "SLEW_HEADING_MINUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_ALTIT_PLUS, "SLEW_ALTIT_PLUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_ALTIT_MINUS, "SLEW_ALTIT_MINUS");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_ALTIT_UP_SLOW, "SLEW_ALTIT_UP_SLOW");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_ALTIT_DN_SLOW, "SLEW_ALTIT_DN_SLOW");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_ALTIT_UP_FAST, "SLEW_ALTIT_UP_FAST");

                    SimConnection.MapClientEventToSimEvent(EVENTS.TOW_PLANE_REQUEST, "TOW_PLANE_REQUEST");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.TOW_PLANE_REQUEST, false);

                    SimConnection.MapClientEventToSimEvent(EVENTS.TOW_PLANE_RELEASE, "TOW_PLANE_RELEASE");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.TOW_PLANE_RELEASE, false);

                    SimConnection.MapClientEventToSimEvent(EVENTS.SITUATION_RESET, "SITUATION_RESET");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.SITUATION_RESET, false);

                    SimConnection.MapClientEventToSimEvent(EVENTS.SITUATION_SAVE, "SITUATION_SAVE");

                    SimConnection.MapClientEventToSimEvent(EVENTS.FREEZE_LATITUDE_LONGITUDE_TOGGLE, "FREEZE_LATITUDE_LONGITUDE_TOGGLE");
                    SimConnection.AddClientEventToNotificationGroup(GROUP_IDS.GROUP_1, EVENTS.FREEZE_LATITUDE_LONGITUDE_TOGGLE, false);

                 }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Problem Initialising Events!\n\n{0}\n\n{1}",
                                          ex.Message, ex.StackTrace);
            }
        }

        private void CloseSimConnection()
        {
            try
            {
                if (SimConnection != null)
                {
                    // listen to connect and quit msgs 
                    //SimConnection.OnRecvOpen += new SimConnect.RecvOpenEventHandler(SimCon_OnRecvOpen);
                    //SimConnection.OnRecvQuit += new SimConnect.RecvQuitEventHandler(SimCon_OnRecvQuit);

                    // listen to exceptions 
                    //SimConnection.OnRecvException += new SimConnect.RecvExceptionEventHandler(SimCon_OnRecvException);

                    // listen to events 
                    //SimConnection.OnRecvEvent += new SimConnect.RecvEventEventHandler(SimCon_OnRecvEvent);

                    // Unsubscribe from all the system events 
                    SimConnection.UnsubscribeFromSystemEvent(EVENTS.FOURSECS);
                    SimConnection.UnsubscribeFromSystemEvent(EVENTS.SIMSTART);
                    SimConnection.UnsubscribeFromSystemEvent(EVENTS.SIMSTOP);
                    SimConnection.UnsubscribeFromSystemEvent(EVENTS.PAUSE);
                    SimConnection.UnsubscribeFromSystemEvent(EVENTS.CRASH_RESET);
                    sim.CloseSimConnection();
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Problem Initialising Events!\n\n{0}\n\n{1}",
                                          ex.Message, ex.StackTrace);
            }
        }

        void SimCon_OnRecvOpen(SimConnect sender, SIMCONNECT_RECV_OPEN data)
        {
            System.Console.WriteLine("Connected to Prepar3D");
        }

        // The case where the user closes Prepar3D 
        void SimCon_OnRecvQuit(SimConnect sender, SIMCONNECT_RECV data)
        {
            System.Console.WriteLine("Prepar3D has exited");
            CloseSimConnection();
        }


        void SimCon_OnRecvException(SimConnect sender, SIMCONNECT_RECV_EXCEPTION data)
        {
            System.Console.WriteLine("Exception received: " + data.dwException);
        }

        async void SimCon_OnRecvEvent(SimConnect sender, SIMCONNECT_RECV_EVENT recEvent)
        {
            switch (recEvent.uEventID)
            {
                case (uint)EVENTS.SIMSTART:
                    System.Console.WriteLine("Sim running");
                    break;

                case (uint)EVENTS.SIMSTOP:
                    System.Console.WriteLine("Sim stopped");
                    break;

                case (uint)EVENTS.FOURSECS:
                    // System.Console.WriteLine("4s tick");
                    // Check current aircraft position
                    {
                        try
                        {
                            string[] cmd = { "position", "current" };
                            dynamic response = await simRest.RunCmdAsync(cmd);
                            Newtonsoft.Json.Linq.JObject jRep = response;
                            if (jRep.Count > 0)
                            {
                                string val1 = (string)jRep["status"];
                                statusOnGround = (1 == (int)jRep["current"]["on_ground"]);
                                currentLatitude = (float)jRep["current"]["latitude"];
                                currentLongitude = (float)jRep["current"]["longitude"];
                            }
                        }
                        catch (Exception ex)
                        {
                        }
                    }
                    UpdateFailureButtons();
                    break;
                case (uint)EVENTS.SITUATION_RESET:
                    System.Console.WriteLine("Situation reset");
                    ClearFailures();
                    Label_Rewind.Content = "";
                    break;
                case (uint)EVENTS.CRASH_RESET:
                    System.Console.WriteLine("Crash reset");
                    ClearFailures();
                    break;
                case (uint)EVENTS.TOW_PLANE_RELEASE:
                    System.Console.WriteLine("Tow plane release");
                    break;
                case (uint)EVENTS.TOW_PLANE_REQUEST:
                    System.Console.WriteLine("Tow plane request");
                     break;
                case (uint)EVENTS.SLEW:
                    System.Console.WriteLine("Slew toggle");
                    break;
                case (uint)EVENTS.SLEW_FREEZE:
                    System.Console.WriteLine("Slew freeze");
                    break;
                case (uint)EVENTS.PAUSE:
                    if (recEvent.dwData == 1)
                    {
                        Button_Pause.Content = "Run";
                        sim.IsPaused = true;
                        System.Console.WriteLine("Sim Paused");
                    }
                    else
                    {
                        Button_Pause.Content = "Pause";
                        sim.IsPaused = false;
                        System.Console.WriteLine("Sim Un-Paused");
                    }
                    RewindConfigure(sim.IsPaused);
                    break;

            }
        }


        void SimCon_OnRecvSimobjectDataBytype(SimConnect sender, SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE data)
        {

            switch ((DataRequestID)data.dwRequestID)
            {
                case DataRequestID.Weather_Data:
                    break;
                case DataRequestID.Location_Data:
                    break;
                default:
                    System.Console.WriteLine("Unknown request ID: " + data.dwRequestID);
                    break;
            }
        }

        void SimCon_OnRevWeatherObservation(SimConnect sender, SIMCONNECT_RECV_WEATHER_OBSERVATION data)
        {
            System.Console.WriteLine("Weather Data: " + data.szMetar);
            sim.CurrentMetar.MetarString = data.szMetar;
            UpdateWeatherControls(sim.CurrentMetar);
            Button_Set_Weather.IsEnabled = false;
            System.Console.WriteLine("Set Weather Data: " + data.szMetar);
            System.Console.WriteLine("Get Weather Data: " + sim.CurrentMetar.MetarString);
        }

#endregion

 
        private void Button_Launch_P3D_Click(object sender, RoutedEventArgs e)
        {
            LaunchP3D();
        }

        private void Button_Kill_PSD_Click(object sender, RoutedEventArgs e)
        {
            CloseSimConnection();
            sim.KillP3D();
            p3DAlreadyRunning = false;
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            try
            {
                if (SimConnection != null)
                {
                    // Sending the title of the Vehicle 
                    SimConnection.ChangeVehicle("Mooney Bravo Retro");
                }

            }
            catch (Exception ex)
            {
                // We were unable to connect so let the user know why. 
                System.Console.WriteLine("Sim Connect unable to connect to Prepar3D!\n\n{0}\n\n{1}",
                                         ex.Message, ex.StackTrace);
            }

        }

        private void Button_Test_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Pause();
            //HRESULT hr = SimConnect_TransmitClientEvent(p3d->getHandle(), SIMCONNECT_OBJECT_ID_USER, event, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }
        
        #region scenarios

        private void ComboAirfields_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            Scenarios_For_Airfield.Clear();
            foreach (Scenario entry in Scenarios)
            {
                string airfield = e.AddedItems[0].ToString();
                if (entry.FileName.Contains(airfield))
                {
                    Scenarios_For_Airfield.Add(entry);
                }
            }

            listScenariosForAirField.Items.Clear();

            foreach (Scenario obj in Scenarios_For_Airfield)
            {
                ListBoxItem itm = new ListBoxItem();
                itm.Content = obj.Title + " : " + obj.Description;
                itm.Tag = obj;
                listScenariosForAirField.Items.Add(itm);
            }
        }

        private void ListScenariosForAirField_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                ListBoxItem toLoad = (ListBoxItem)e.AddedItems[0];
                selectedScenario = (Scenario)toLoad.Tag;
            }
            else
                selectedScenario = null;
        }

        private void Button_Load_Scenario_Click(object sender, RoutedEventArgs e)
        {
            if (selectedScenario != null)
            {
                String scenarioFileName = selectedScenario.FileName;
                if (SimConnection != null)
                {
                    simRest.CMD_Position_Clear_History();
                    SimConnection.FlightLoad(scenarioFileName);
                    ClearFailures();
                }
            }

        }

        private void BuildScenarioLists()
        {
            var value = System.Environment.GetEnvironmentVariable("USERPROFILE");
            string path = value + "\\Documents\\Prepar3D v4 Files";
            string[] fileArray = Directory.GetFiles(@path, "*.fxml");
            foreach (string fileName in fileArray)
            {
                Console.WriteLine(fileName);
            }

            // Parse file names rejecting any that don't start with "IOS_"
            Scenarios.Clear();
            Scenario_Airfields.Clear();
            defaultScenario = "";
            defaultAirfield = "";

            foreach (string fileName in fileArray)
            {
                string strScenario = fileName.Substring(path.Length + 1);
                if (strScenario.StartsWith("IOS_"))
                {
                    Scenario scenario = new Scenario(fileName);
                    Scenarios.Add(scenario);

                    char[] delimiterChars = { '_' };

                    string[] words = strScenario.Split(delimiterChars);

                    string airfield = words[1];
                    bool bAlreadyInList = false;
                    foreach (var entry in Scenario_Airfields)
                    {
                        if (entry == airfield)
                        {
                            bAlreadyInList = true;
                        }
                    }
                    if (bAlreadyInList == false)
                    {
                        Scenario_Airfields.Add(airfield);
                    }

                    if (strScenario.Contains("_DEFAULT_"))
                    {
                        defaultScenario = strScenario;
                        defaultAirfield = airfield;
                    }

                }
            }
            comboAirfields.ItemsSource = Scenario_Airfields;
            comboAirfields.SelectedItem = defaultAirfield;

         }


        #endregion

        #region weather

        private void Button_Weather_Test_Click(object sender, RoutedEventArgs e)
        {
            if (SimConnection != null)
                sim.WeatherRequest(currentLatitude, currentLongitude);
        }

        private void Window_Initialized(object sender, EventArgs e)
        {
        }

        private void Window_Activated(object sender, EventArgs e)
        {
            // Called when window actual exists
            if (handle == IntPtr.Zero)
            {
                handle = new WindowInteropHelper(this).Handle; // Get handle of main WPF Window
                handleSource = HwndSource.FromHwnd(handle); // Get source of handle in order to add event handlers to it
                handleSource.AddHook(HandleSimConnectEvents);
                BuildScenarioLists();
                LaunchP3D();
            }
        }

        private async void BuildWeatherThemesList()
        {
            try
            {
                weatherThemes.Clear();
                string[] cmd = { "weather", "themes" };
                dynamic response = await simRest.RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    Newtonsoft.Json.Linq.JToken jToken = jRep["themes"];
                    var themes = jToken.Children();
                    foreach (var item in themes)
                    {
                        string name = (string)item["name"];
                        string title = (string)item["title"];
                        string description = (string)item["description"];
                        WeatherTheme wItem = new WeatherTheme(name, title, description);
                        weatherThemes.Add(wItem);
                    }

                }
                listWeatherThemes.ItemsSource = weatherThemes;
                listWeatherThemes.DisplayMemberPath = "Text";
                listWeatherThemes.SelectedValuePath = "Name";
            }
            catch (Exception ex)
            {
            }
        }

        private void ListWeatherThemes_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //Button_Set_Weather_Theme.IsEnabled = true;
        }


        private void Button_Set_Weather_Click(object sender, RoutedEventArgs e)
        {
            sim.WeatherSetWind();
            Button_Set_Weather.IsEnabled = false;

        }


        private void UpdateWeatherControls(Metar metar)
        {
            sldWind_0_Dir.Value = metar.SurfaceWindDirection;
            sldWind_0_Spd.Value = metar.SurfaceWindSpeed;
            Button_Set_Weather.IsEnabled = true;
        }

        private void SldWind_0_Spd_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            sim.CurrentMetar.SurfaceWindSpeed = (int)e.NewValue;
            Button_Set_Weather.IsEnabled = true;
        }

        private void SldWind_0_Gst_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //sim.CurrentMetar.SurfaceWindSpeed = (int)e.NewValue;
            Button_Set_Weather.IsEnabled = true;
        }

        private void SldWind_0_Dir_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            sim.CurrentMetar.SurfaceWindDirection = (int)e.NewValue;
            Button_Set_Weather.IsEnabled = true;
        }

        #endregion 

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            System.Windows.Controls.TabControl tabControl = sender as System.Windows.Controls.TabControl; // e.Source could have been used instead of sender as well
            TabItem item = tabControl.SelectedValue as TabItem;
            if (item.Name == "TabWeather")
            {
                if (SimConnection != null)
                {
                    label_gust.Visibility = Visibility.Hidden;
                    lblWind_0_Gst.Visibility = Visibility.Hidden;
                    sldWind_0_Gst.Visibility = Visibility.Hidden;
                    sim.WeatherRequest(currentLatitude, currentLongitude);
                    //BuildWeatherThemesList();
                }
            }
            else if (item.Name == "TabTraffic")
            {
                if (SimConnection != null)
                {
                    InitialiseTrafficTab();
                }
            }
        }

        #region slewing

        private void Button_Pause_Click(object sender, RoutedEventArgs e)
        {
             SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
        }

        public void SlewStart()
        {
            if (!simPausedOnStartingSlew && sim.IsPaused)
            {
                simPausedOnStartingSlew = sim.IsPaused;
                SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
            }
            if (!simSlewing)
            {
                SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
                simSlewing = true;
            }
        }

        public void SlewStop()
        {
            simSlewing = false;
            if (simPausedOnStartingSlew)
            {
                SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
                simPausedOnStartingSlew = false;
            }
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_FREEZE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
    }

        private void Button_Slew_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
        }

        private void Button_Slew_Left_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_LEFT, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);

        }

        private void Button_Slew_Forward_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_AHEAD_PLUS, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Slew_Right_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_RIGHT, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Slew_Backwards_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_AHEAD_MINUS, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Slew_Freeze_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStop();
        }

        private void Button_Slew_Rotate_Leftt_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_HEADING_PLUS, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Slew_Up_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_ALTIT_UP_SLOW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Slew_Down_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            SlewStart();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW_ALTIT_DN_SLOW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        #endregion

        private void Button_Request_Winch_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Position_Clear_History();
            //statusWinchLaunchRequested = true;
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.TOW_PLANE_RELEASE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Request_Aerotow_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Position_Clear_History();
            //statusAerotowRequested = true;
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.TOW_PLANE_REQUEST, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Release_Cable_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Position_Clear_History();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.TOW_PLANE_RELEASE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Reset_Flight_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Position_Clear_History();
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SITUATION_RESET, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Save_Snapshot_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.FlightSave("Temp", "Snapshot", "", 0);
        }

        private void Button_Recall_Snapshot_Click(object sender, RoutedEventArgs e)
        {
            simRest.CMD_Position_Clear_History();
            SimConnection.FlightLoad("Temp");
        }

#region PositionRewind 

        private async void RewindConfigure(bool paused)
        {
            try
            {
                if (paused == false && rewindCount != 1)
                {
                    // user has rewound the position
                    simRest.CMD_Position_Set(rewindCount);
                    Label_Rewind.Content = "";
                }
                Button_Position_Back.IsEnabled = paused;
                Button_Position_Forward.IsEnabled = paused;
                Slider_Position_Rewind.IsEnabled = paused;
                if (paused)
                {
                    int length = await simRest.CMD_Position_Available();
                    rewindCount = 1;
                    Slider_Position_Rewind.Value = 1;
                    Slider_Position_Rewind.Minimum = 1;
                    Slider_Position_Rewind.Maximum = length;
                    Button_Position_Forward.IsEnabled = false;
                    Label_Rewind.Content = "Rewind : " + (rewindCount - 1).ToString() + " seconds";

                }
            }
            catch (Exception ex)
            {
                // We were unable to connect so let the user know why. 
                System.Console.WriteLine("Rewind Configure\n\n{0}\n\n{1}",
                                         ex.Message, ex.StackTrace);
            }
        }


        private void Button_Position_Back_Click(object sender, RoutedEventArgs e)
        {
            rewindCount += 1;
            if (rewindCount > (int)Slider_Position_Rewind.Maximum-1)
            {
                Button_Position_Back.IsEnabled = false;
                rewindCount = (int)Slider_Position_Rewind.Maximum - 1;
            }
            Button_Position_Forward.IsEnabled = true;
           // rewindCount = Math.Max(0, rewindCount);
            Slider_Position_Rewind.Value = rewindCount;
            simRest.CMD_Position_Back(rewindCount);
        }

        private void Button_Position_Forward_Click(object sender, RoutedEventArgs e)
        {
            rewindCount -= 1;
            if (rewindCount<1)
            {
                Button_Position_Forward.IsEnabled = false;
            }
            Button_Position_Back.IsEnabled = true;
            rewindCount = Math.Max(1, rewindCount);
            Slider_Position_Rewind.Value = rewindCount;
            simRest.CMD_Position_Back(rewindCount);
        }

        private void Slider_Position_Rewind_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (rewindCount != (int)e.NewValue)
            {
                rewindCount = (int)e.NewValue;
                rewindActive = true;
            }
            Label_Rewind.Content = "Rewind : " + (rewindCount - 1).ToString() + " seconds";
        }

        private void Slider_Position_Rewind_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            if (rewindActive)
            {
                rewindActive = false;
                simRest.CMD_Position_Back(rewindCount);
            }
        }

        #endregion

        private void Button_Freeze_Position_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.FREEZE_LATITUDE_LONGITUDE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        #region Failures

        private void ClearFailures()
        {
            string[] cmd = { "failures", "clear" };
            simRest.RunCmdAsync(cmd);
        }

        private async void UpdateFailureButtons()
        {
            bool changed = await simRest.Get_Failure_States();
            if (changed)
            {
                Button_Fail_ASI.Content = simRest.ASIFailed ? "Clear ASI" : "Fail ASI";
                //Button_Fail_ASI.Background.SetCurrentValue()
                Button_Fail_Altimeter.Content = simRest.AltimeterFailed ? "Clear Altimeter" : "Fail Altimeter";
                Button_Fail_Pitot.Content = simRest.PitotFailed ? "Clear Pitot" : "Fail Pitot";
                Button_Fail_Electrical.Content = simRest.ElectricsFailed ? "Clear Electrics" : "Fail Electrics";
            }
        }

        private async void Button_Fail_ASI_Click(object sender, RoutedEventArgs e)
        {
            string mode = simRest.ASIFailed ? "false" : "true";
            string[] cmd = { "failures", "airspeed","mode=" + mode };
            await simRest.RunCmdAsync(cmd);
            UpdateFailureButtons();
        }

        private async void Button_Fail_Altimeter_Click(object sender, RoutedEventArgs e)
        {
            string mode = simRest.AltimeterFailed ? "false" : "true";
            string[] cmd = { "failures", "altimeter", "mode=" + mode };
            await simRest.RunCmdAsync(cmd);
            UpdateFailureButtons();
        }

        private async void Button_Fail_Pitot_Click(object sender, RoutedEventArgs e)
        {
            string mode = simRest.PitotFailed ? "false" : "true";
            string[] cmd = { "failures", "pitot", "mode=" + mode };
            await simRest.RunCmdAsync(cmd);
            UpdateFailureButtons();
        }

        private async void Button_Fail_Electrical_Click(object sender, RoutedEventArgs e)
        {
            string mode = simRest.ElectricsFailed ? "false" : "true";
            string[] cmd = { "failures", "electrical", "mode=" + mode };
            await simRest.RunCmdAsync(cmd);
            UpdateFailureButtons();
        }

        #endregion

        private void InitialiseTrafficTab()
        {
            if (Traffic_Aircraft.Count == 0)
            {
                comboTrafficSpeed.ItemsSource = trafficSpeeds;
                comboTrafficSpeed.DisplayMemberPath = "Description";
                comboTrafficSpeed.SelectedValuePath = "Speed";
                comboTrafficSpeed.SelectedIndex = 3;

                comboTrafficRange.ItemsSource = trafficRanges;
                comboTrafficRange.DisplayMemberPath = "Description";
                comboTrafficRange.SelectedValuePath = "Range";
                comboTrafficRange.SelectedIndex = 2;

                comboTrafficHeight.ItemsSource = trafficHeights;
                comboTrafficHeight.DisplayMemberPath = "Description";
                comboTrafficHeight.SelectedValuePath = "RelativeHeight";
                comboTrafficHeight.SelectedIndex = 4;

                comboTrafficRelativeHeading.ItemsSource = trafficHeadings;
                comboTrafficRelativeHeading.DisplayMemberPath = "Description";
                comboTrafficRelativeHeading.SelectedValuePath = "RelativeHeading";
                comboTrafficRelativeHeading.SelectedIndex = 18;

                BuildAircraftList();
            }
        }

        private async void BuildAircraftList()
        {
            try
            {
                Traffic_Aircraft.Clear();
                string[] cmd = { "traffic", "aircraft" };
                dynamic response = await simRest.RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    Newtonsoft.Json.Linq.JToken jToken = jRep["aircraft"];
                    //var aircraft = jToken["title"];
                    var aircraftTypes = jToken.Children();
                    foreach (var item in aircraftTypes)
                    {
                        string type = (string)item["title"];
                        Traffic_Aircraft.Add(type);
                    }

                }
            }
            catch (Exception ex)
            {
            }
            comboTrafficAircraft.ItemsSource = Traffic_Aircraft;
            comboTrafficAircraft.SelectedIndex = 0;
        }

        private async void Button_Traffic_Launch_Click(object sender, RoutedEventArgs e)
        {
            string traffic_Aircraft = (string)comboTrafficAircraft.SelectedItem;
            TrafficSpeed speed = (TrafficSpeed)comboTrafficSpeed.SelectedItem;
            TrafficRange range = (TrafficRange)comboTrafficRange.SelectedItem;
            TrafficHeight height = (TrafficHeight)comboTrafficHeight.SelectedItem;
            TrafficHeading heading = (TrafficHeading)comboTrafficRelativeHeading.SelectedItem;

            string[] cmd = { "traffic", "launch", "name=" + traffic_Aircraft,
                "range=" + range.Distance.ToString(),
                "speed=" + speed.Speed.ToString(),
                "relative_height="  + height.RelativeHeight.ToString(),
                "bearing=" + heading.RelativeHeading.ToString() };

            dynamic response = await simRest.RunCmdAsync(cmd);
        }

        private void ComboTrafficAircraft_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //traffic_Aircraft = e.AddedItems[0].ToString();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            MessageBoxResult result = System.Windows.MessageBox.Show("Do you really want to close the simulator?", "Warning", MessageBoxButton.YesNo);
            if (result != MessageBoxResult.Yes)
            {
                e.Cancel = true;
            }
            else
            {
                closeWinchXDialog.Abort();
                minimiseP3DControl.Abort();
                minimiseP3DInstruments1.Abort();
                minimiseP3DToPDA.Abort();
            }
        }
    }

    public class TextBoxOutputter : TextWriter
    {
        System.Windows.Controls.TextBox textBox = null;

        public TextBoxOutputter(System.Windows.Controls.TextBox output)
        {
            textBox = output;
        }

        public override void Write(char value)
        {
            base.Write(value);
            textBox.Dispatcher.BeginInvoke(new Action(() =>
            {
                textBox.AppendText(value.ToString());
            }));
        }

        public override Encoding Encoding
        {
            get { return System.Text.Encoding.UTF8; }
        }
    }

    public class TrafficSpeed
    {
        public int Speed { get; set; }

        public string Description { get; set; }

        public TrafficSpeed(int speed)
        {
            Speed = speed;
            Description = Speed.ToString() + " Knots";
        }
    }

    public class TrafficRange
    {
        public float Distance { get; set; }

        public string Description { get; set; }

        public TrafficRange(float distance)
        {
            Distance = distance;
            Description = distance.ToString() + " Miles";
        }
    }

    public class TrafficHeight
    {
        public int RelativeHeight { get; set; }

        public string Description { get; set; }

        public TrafficHeight(int relHeight)
        {
            RelativeHeight = relHeight;
            Description = relHeight.ToString() + " Feet";
        }
    }

    public class TrafficHeading
    {
        public int RelativeHeading { get; set; }

        public string Description { get; set; }

        public TrafficHeading(int relHeading)
        {
            RelativeHeading = relHeading;
            if (relHeading < 0)
                Description = relHeading.ToString() + " Degrees (from the left)";
            else if (relHeading == 0)
                Description = "Head on";
            else
                Description = relHeading.ToString() + " Degrees (from the right)";
        }
    }

    public class WeatherTheme
    {
        public string Name { get; set; }

        public string Description { get; set; }
        public string Title { get; set; }
        public string Text
        {
            get { return Title + " : " + Description; }
        }

        public WeatherTheme(string name, string title, string description)
        {
            Name = name;
            Title = title;
            Description = description;
        }
    }

}

