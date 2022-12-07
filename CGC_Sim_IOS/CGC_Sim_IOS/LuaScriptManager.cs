using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;

namespace CGC_Sim_IOS
{
    public enum LuaScriptType
    {
        Undefined,
        Aerotow,
    }

    class LuaScriptManager
    {
        private ObservableCollection<LuaScriptFile> scriptFiles = new ObservableCollection<LuaScriptFile>();
        private ObservableCollection<LuaScriptFile> scriptAerotowFiles = new ObservableCollection<LuaScriptFile>();

        public ObservableCollection<LuaScriptFile> LuaScriptFiles
        {
            get
            {
                return scriptFiles;
            }
        }

        public ObservableCollection<LuaScriptFile> AerotowScriptFiles
        {
            get
            {
                return scriptAerotowFiles;
            }
        }

        public void BuildScriptList()
        {
            try
            {
                var value = System.Environment.GetEnvironmentVariable("USERPROFILE");
                string path;
#if (P3Dv5)
                path = value + "\\Documents\\Prepar3D v5 Files\\Script";
#else
                path = value + "\\Documents\\Prepar3D v4 Files\\Script";
#endif
                string[] fileArray = System.IO.Directory.GetFiles(@path, "*.lua");
                foreach (string fileName in fileArray)
                {
                    Console.WriteLine(fileName);
                }

                // Parse file names rejecting any that don't start with "IOS_"
                scriptFiles.Clear();
                scriptAerotowFiles.Clear();

                foreach (string fileName in fileArray)
                {
                    string strScript = fileName.Substring(path.Length + 1);
                    if (strScript.StartsWith("IOS_"))
                    {
                        char[] delimiterChars = { '_' };

                        string[] words = strScript.Split(delimiterChars);
                        if (words.Count() > 2)
                        {
                            if (words[1] == "Aerotow")
                            {
                                LuaScriptFile item = new LuaScriptFile(strScript, words[2], LuaScriptType.Aerotow);
                                scriptAerotowFiles.Add(item);
                                scriptFiles.Add(item);
                            }
                            else
                            {
                                LuaScriptFile item = new LuaScriptFile(strScript, words[2]);
                                scriptFiles.Add(item);
                            }

                        }
                    }
                }
            }
            catch (Exception ex)
            { 
            }
        }

    }

    public class LuaScriptFile
    {
        public string FileName { get; set; }

        public string Description { get; set; }

        public LuaScriptType Type { get; set; }

        public LuaScriptFile(string name, string description, LuaScriptType type = LuaScriptType.Undefined)
        {
            FileName = name;
            Description = description;
            Type = type;
        }
    }


}
