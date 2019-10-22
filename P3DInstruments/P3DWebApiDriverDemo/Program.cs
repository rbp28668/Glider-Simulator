using System;
using System.Threading.Tasks;
using System.Net.Http;
using System.Net;
using Newtonsoft.Json;  // from nuget see https://www.newtonsoft.com/json/help/html/Introduction.htm



namespace P3DWebApiDriverDemo
{
    class P3dWebApi
    {
        // Should create a single instance of HttpClient and re-use it.
        // See https://docs.microsoft.com/en-us/dotnet/api/system.net.http.httpclient?view=netframework-4.8
        HttpClient client = new HttpClient();

        void Run(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Need to supply at least the command and its sub-command");
                return;
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
                    url += WebUtility.UrlEncode(args[index++]);
                }

                while (index < args.Length)
                {
                    url += "&";
                    url += WebUtility.UrlEncode(args[index++]);
                }

                // Should have a complete URL with all the parameters
                Console.WriteLine(url);


                string responseBody = Run(url).Result;

                Console.WriteLine(responseBody);

                parseResult(responseBody);
                
            }
            catch (HttpRequestException e)
            {
                Console.WriteLine("\nException Caught!");
                Console.WriteLine("Message :{0} ", e.Message);
            }
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

        void parseResult(string json)
        {
            dynamic data = JsonConvert.DeserializeObject(json);
            bool isOK = data.status == "OK";
            if (isOK)
            {
                // do somethig creative here....
            }
        }

        static void Main(string[] args)
        {
            P3dWebApi p = new P3dWebApi();
            p.Run(args);
        }
    }
}
