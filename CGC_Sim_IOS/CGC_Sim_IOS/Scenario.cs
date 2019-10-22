using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

namespace CGC_Sim_IOS
{
    class Scenario
    {
        public string FileName { get; set; } = "";
        public string Title { get; set; } = "";
        public string Description { get; set; } = "";

        public Scenario (string scenarioFilename)
        {
            XElement xmlScenario = XElement.Load(scenarioFilename);

            IEnumerable<XElement> el = from xElement in xmlScenario.Descendants("Section")
                                       where xElement.FirstAttribute.Value == "Main"
                                       select xElement;

            IEnumerable<XElement> description = from xElement in el.Descendants("Property")
                                          where xElement.FirstAttribute.Value == "Description"
                                          select xElement;

            IEnumerable<XElement> title = from xElement in el.Descendants("Property")
                                          where xElement.FirstAttribute.Value == "Title"
                                          select xElement;

            FileName = scenarioFilename;
            Title = (string)title.ElementAt(0).LastAttribute.Value;
            Description = (string)description.ElementAt(0).LastAttribute.Value;
           
        }
    }
}
