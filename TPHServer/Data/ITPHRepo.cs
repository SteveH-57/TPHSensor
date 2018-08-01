using System.Collections.Generic;
using System.Threading.Tasks;
using TPHServer.Models;

namespace TPHServer.Data
{
    public interface ITPHRepo
    {
        Task<ITPHReading> AddReading(ITPHReading reading);
        Task<IEnumerable<IDevice>> GetAccountDevices(string account);
        Task<IEnumerable<IAggregateRecord>> GetAggregates(string deviceId, string type, string first, string last);
        IEnumerable<ITPHReading> GetDeviceReadings(string deviceId);
    }
}