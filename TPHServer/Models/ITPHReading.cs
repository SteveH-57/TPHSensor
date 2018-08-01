using System;

namespace TPHServer.Models
{
    public interface ITPHReading
    {
        string Id { get; set; }
        double Humidity { get; set; }
        double Pressure { get; set; }
        double Temperature { get; set; }
        DateTime ReadAt { get; set; }
    }
}