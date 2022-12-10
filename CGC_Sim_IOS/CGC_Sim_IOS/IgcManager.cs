using System;
using System.IO;

using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;

namespace CGC_Sim_IOS
{
    class IgcManager
    {
        private ObservableCollection<IGCFile> igcFiles = new ObservableCollection<IGCFile>();
        private string currentAirfield = "";
        public ObservableCollection<IGCFile> IGCFiles
        {
            get
            {
                return igcFiles;
            }
        }

        public async Task BuildIGCTrafficList(SimRestConnection simRest,string targetAirfield)
        {
            if (currentAirfield != targetAirfield)
            {
                igcFiles.Clear();
                currentAirfield = targetAirfield;
                string[] cmd = { "igc", "list" };
                dynamic response = await simRest.RunCmdAsync(cmd);
                Newtonsoft.Json.Linq.JObject jRep = response;
                if (jRep.Count > 0)
                {
                    string val1 = (string)jRep["status"];
                    Newtonsoft.Json.Linq.JToken jToken = jRep["entries"];
                    //var aircraft = jToken["title"];
                    var files = jToken.Children();
                    foreach (var item in files)
                    {
                        string fileName = (string)item["filename"];
                        fileName = fileName.Trim();

                        if (fileName.StartsWith("IOS_" + targetAirfield + "_"))
                        {
                            Console.WriteLine(fileName);
                            char[] delimiterChars = { '_' };
                            string[] words = fileName.Split(delimiterChars);
                            string description = words[2];
                            description = description.Substring(0, description.Length - 4);
                            IGCFile igc = new IGCFile(fileName, description);
                            igcFiles.Add(igc);
                        }
                    }
                }
            }
        }

        public bool LaunchIGCTraffic(SimRestConnection simRest, IGCFile igcFile, bool loop)
        {
            String fileName = igcFile.FileName;
            String aircraftType = "ASK21_Simulator";
            if (loop)
            {
                string[] cmd = { "igc", "traffic", "igc=" + fileName, "type=" + aircraftType, "restart=true" };
                simRest.RunCmdAsync(cmd);
            }
           else
            {
                string[] cmd = { "igc", "traffic", "igc=" + fileName, "type=" + aircraftType, "restart=false"};
                simRest.RunCmdAsync(cmd);
            }

            return true;
        }

        public void ClearTraffic(SimRestConnection simRest)
        {
            string[] cmd = { "igc", "clear"};
            simRest.RunCmdAsync(cmd);
        }

        

    }

public class IGCFile
    {
        public string FileName { get; set; }

        public string Description { get; set; }

        public bool Loop { get; set; }

        public IGCFile(string name, string description)
        {
            FileName = name;
            Description = description;
            Loop = false;
        }
    }

}
