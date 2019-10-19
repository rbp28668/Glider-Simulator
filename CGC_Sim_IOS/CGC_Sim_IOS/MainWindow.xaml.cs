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

namespace CGC_Sim_IOS
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        [DllImport("user32.dll")]
        static extern bool SetForegroundWindow(IntPtr hWnd);

        private Simulator sim = new Simulator();

        private List<Scenario> Scenarios = new List<Scenario>();
        private List<String> Scenario_Airfields = new List<String>();
        private List<Scenario> Scenarios_For_Airfield = new List<Scenario>();
        private string defaultScenario = "";
        private string defaultAirfield = "";

        private Scenario selectedScenario = null;
        private bool p3DAlreadyRunning = false;

        private bool simPausedOnStartingSlew = false;

        private Thread closeWinchXDialog;

        const uint SIMCONNECT_OBJECT_ID_USER = 0;           // proxy value for User vehicle ObjectID
        const uint DATA = 0;


        public SimConnect SimConnection
        {
            get
            {
                if (sim.SimConnection == null)
                {
                    if (p3DAlreadyRunning)
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
 //           Console.SetOut(outputter);

        }

        ~MainWindow()
        {
            if (handleSource != null)
            {
                handleSource.RemoveHook(HandleSimConnectEvents);
            }
        }

        private void closeWinchXDialogTask(object obj)
        {
            bool closed = false;
            int loop = 0;
            while (closed == false && loop < 10)
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
                }
                Thread.Sleep(5000);
                loop++;
            }
        }

        //void ActivateApp(string processName)
        //{
        //    Process[] p = Process.GetProcessesByName(processName);

        //    // Activate the first application we find with this name
        //    if (p.Count() > 0)
        //        SetForegroundWindow(p[0].MainWindowHandle);
        //}

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

                    // Initially turn the events off 
                    SimConnection.SetSystemEventState(EVENTS.FOURSECS, SIMCONNECT_STATE.OFF);
                    SimConnection.SetSystemEventState(EVENTS.SIMSTART, SIMCONNECT_STATE.ON);
                    SimConnection.SetSystemEventState(EVENTS.SIMSTOP, SIMCONNECT_STATE.ON);

                    SimConnection.OnRecvSimobjectDataBytype += new SimConnect.RecvSimobjectDataBytypeEventHandler(SimCon_OnRecvSimobjectDataBytype);
                    SimConnection.MapClientEventToSimEvent(EVENTS.PAUSE_TOGGLE, "PAUSE_TOGGLE");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW, "SLEW_TOGGLE");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SLEW_FREEZE, "SLEW_FREEZE");
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
                    SimConnection.MapClientEventToSimEvent(EVENTS.TOW_PLANE_RELEASE, "TOW_PLANE_RELEASE");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SITUATION_RESET, "SITUATION_RESET");
                    SimConnection.MapClientEventToSimEvent(EVENTS.SITUATION_SAVE, "SITUATION_SAVE");



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

        void SimCon_OnRecvEvent(SimConnect sender, SIMCONNECT_RECV_EVENT recEvent)
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
                    System.Console.WriteLine("4s tick");
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
                    //Struct1 s1 = (Struct1)data.dwData[0];

                    //displayText("title: " + s1.title);
                    //displayText("Lat:   " + s1.latitude);
                    //displayText("Lon:   " + s1.longitude);
                    //displayText("Alt:   " + s1.altitude);
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

        private void LaunchP3D()
        {
            Process[] p = Process.GetProcessesByName("Prepar3D");
            if (p.Count() == 0)
            {
                sim.StartP3D(defaultScenario);
                closeWinchXDialog = new Thread(closeWinchXDialogTask);
                closeWinchXDialog.Start();
                p3DAlreadyRunning = true;
            }
            if (!p3DAlreadyRunning)
            {
                closeWinchXDialog = new Thread(closeWinchXDialogTask);
                closeWinchXDialog.Start();
                p3DAlreadyRunning = true;
            } 
        }

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
            //HRESULT hr = SimConnect_TransmitClientEvent(p3d->getHandle(), SIMCONNECT_OBJECT_ID_USER, event, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }

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
                    SimConnection.FlightLoad(scenarioFileName);
            }

        }

        private void Button_Weather_Test_Click(object sender, RoutedEventArgs e)
        {
            if (SimConnection != null)
                sim.WeatherRequest();
        }

        private void Window_Initialized(object sender, EventArgs e)
        {
        }

        private void Window_Activated(object sender, EventArgs e)
        {
            // Called when window actual exists
            if (handle == IntPtr.Zero)
            {
                //AddHandler(FrameworkElement.MouseDownEvent, new MouseButtonEventHandler(Button_Slew_Left_MouseLeftButtonDown), true);
                // AddHandler(FrameworkElement.MouseUpEvent, new MouseButtonEventHandler(Button_Slew_Left_MouseLeftButtonUp), true);
                handle = new WindowInteropHelper(this).Handle; // Get handle of main WPF Window
                handleSource = HwndSource.FromHwnd(handle); // Get source of handle in order to add event handlers to it
                handleSource.AddHook(HandleSimConnectEvents);
                BuildScenarioLists();
            }
            LaunchP3D();

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

        private void SldWind_0_Dir_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            sim.CurrentMetar.SurfaceWindDirection = (int)e.NewValue;
            Button_Set_Weather.IsEnabled = true;
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            System.Windows.Controls.TabControl tabControl = sender as System.Windows.Controls.TabControl; // e.Source could have been used instead of sender as well
            TabItem item = tabControl.SelectedValue as TabItem;
            if (item.Name == "TabWeather")
            {
                if (SimConnection != null)
                {
                    sim.WeatherRequest();
                }
            }
        }

        private void Button_Pause_Click(object sender, RoutedEventArgs e)
        {
             SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
        }

        public void SlewStart()
        {
            simPausedOnStartingSlew = sim.IsPaused;
            if (simPausedOnStartingSlew)
            {
                SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
            }
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
        }

        public void SlewStop()
        {
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

        private void Button_Request_Aerotow_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.TOW_PLANE_REQUEST, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Release_Aerotow_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.TOW_PLANE_RELEASE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Reset_Flight_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SITUATION_RESET, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);
        }

        private void Button_Save_Snapshot_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.FlightSave("Temp", "Snapshot", "", 0);
        }

        private void Button_Recall_Snapshot_Click(object sender, RoutedEventArgs e)
        {
            SimConnection.FlightLoad("Temp");
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

    //class SlewPauser : Object
    //{
    //    private bool isPaused = false;

    //    const uint SIMCONNECT_OBJECT_ID_USER = 0;           // proxy value for User vehicle ObjectID
    //    const uint DATA = 0;
    //    private Simulator Sim;

    //    public SlewPauser(Simulator sim)
    //    {
    //        Sim = sim;
    //        isPaused = Sim.IsPaused;
    //        if (isPaused)
    //        {
    //            Sim.SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
    //        }
    //        Sim.SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
    //    }

    //    ~SlewPauser()
    //    {
    //        isPaused = Sim.IsPaused;
    //        if (isPaused)
    //        {
    //            Sim.SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.PAUSE_TOGGLE, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
    //        }
    //        Sim.SimConnection.TransmitClientEvent(SIMCONNECT_OBJECT_ID_USER, EVENTS.SLEW, DATA, GROUP_IDS.GROUP_1, SIMCONNECT_EVENT_FLAG.GROUPID_IS_PRIORITY);            //% USERPROFILE %\Documents\Prepar3D v4 Files
    //    }
    //}

 }

