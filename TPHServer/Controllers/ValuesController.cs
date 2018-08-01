using Microsoft.AspNetCore.Cors;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using TPHServer.Data;
using TPHServer.Models;


namespace TPHServer.Controllers
{
    //[EnableCors("MyPolicy")]
    [Route("api/[controller]")]
    public class ValuesController : Controller
    {
        ITPHRepo _repo;
        public ValuesController(ITPHRepo repo)
        {
            _repo = repo;
        }


        [HttpGet("[action]")]
        public string Get()
        {
            return "Hello from the other side.";
        }


        /// <summary>
        /// Gets the readings for the specified device.
        /// </summary>
        /// <param name="id">The device id (mac address).</param>
        /// <returns>A list of TPH readings.</returns>
        [ResponseCache(NoStore = true, Location = ResponseCacheLocation.None)]
        [HttpGet("{id}")]
        public IEnumerable<ITPHReading> Get(string id)
        {
            var results = _repo.GetDeviceReadings(id);
            foreach (var record in results)
            {
                yield return record;
            }
        }

        /// <summary>
        /// Get the devices for the specified account.
        /// </summary>
        /// <param name="id">The account id.</param>
        /// <returns>List of devices</returns>
        [ResponseCache(NoStore = true, Location = ResponseCacheLocation.None)]
        [HttpGet("account/{id}")]
        public async Task<IEnumerable<IDevice>> Account(string id)
        {


            try
            {
                return await _repo.GetAccountDevices(id);
            }
            catch (Exception ex)
            {
                var list = new List<IDevice>();
                list.Add( new Device { Account= ex.Source, DeviceId=ex.Message, Active=false, Description= ex.InnerException.ToString(), Location=ex.ToString() });
                return list;
            };

        }


        /// <summary>
        /// Get the aggregate values
        /// </summary>
        /// <param name="id">The device identifier (mac address).</param>
        /// <param name="type">The type (h - hour, d - day, m - month). </param>
        /// <param name="first">The start of the time range. (type h time-range is yyMMddhh) </param>
        /// <param name="last">The end of the time range.</param>
        /// <returns>List of Aggregate records.</returns>
        [HttpGet("aggregate/{id}/{type}/{first}/{last}")]
        public async Task<IEnumerable<IAggregateRecord>> Aggregate(string id, string type, string first, string last)
        {

            var tasks = new List<Task<IEnumerable<IAggregateRecord>>>();
            var result = new List<IAggregateRecord>();
            if (id.Equals("ALL"))
            {
                var devices = await _repo.GetAccountDevices("Temp");
                foreach (var deviceId in devices.Select(d => d.DeviceId)) {
                    tasks.Add(_repo.GetAggregates(deviceId, type, first, last));
                };
            } else
            {
                tasks.Add(_repo.GetAggregates(id, type, first, last));
            }
            await Task.WhenAll(tasks);
            foreach(var t in tasks)
            {
                result.AddRange(t.Result);
            }
            Console.WriteLine($"Aggregate for {id}-{type} from {first} to {last}.");
            return result;
        }

        /// <summary>
        /// Posts the specified temperature, pressure and humidity reading.
        /// </summary>
        /// <param name="T">The temperature.</param>
        /// <param name="P">The pressure.</param>
        /// <param name="H">The humidity.</param>
        /// <param name="I">The device id (mac address).</param>
        /// <param name="O">The time offset in seconds.</param>
        /// <returns>Asynchronous Action result with saved TPHReading values.</returns>
        /// <exception cref="ArgumentException">
        /// </exception>
        [HttpPost]
        public async Task<ActionResult> Post([FromForm]string T, [FromForm]string P, [FromForm]string H, [FromForm]string I, [FromForm]string O)
        {
            try
            {
                if (I is null || T is null || P is null || H is null) { return new BadRequestResult(); }
                double temperature = 0;
                double pressure = 0;
                double humidity = 0;
                long offset = 0;
                if (!double.TryParse(T, out temperature)) throw new ArgumentException();
                if (!double.TryParse(P, out pressure)) throw new ArgumentException();
                if (!double.TryParse(H, out humidity)) throw new ArgumentException();
                long.TryParse(O, out offset);
                var reading = new TPHReading() { Id = I, Temperature = temperature, Pressure = pressure, Humidity = humidity, ReadAt = DateTime.Now.AddSeconds(-offset) };
                Console.WriteLine(reading);
                return Ok( await _repo.AddReading(reading));
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                Console.WriteLine($"Rec'd: T='{T}'  P='{P}'  H='{H}'  I='{I}'  O='{O}';");
                return new BadRequestResult();
            }
        }
    }

}
