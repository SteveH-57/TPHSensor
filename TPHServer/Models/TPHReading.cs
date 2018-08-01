using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace TPHServer.Models
{
    public class TPHReading : ITPHReading
    {
        public string Id { get; set; }
        public double Humidity { get; set; }
        public double Pressure { get; set; }
        public double Temperature { get; set; }
        public DateTime ReadAt { get; set; }

        public override string ToString()
        {
            return $"Device: {Id,-23} Temp: {Temperature:00.00} Pres: {Pressure:00.00} Humid: {Humidity:00.00}";
        }
    }
   
}
