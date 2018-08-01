using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace TPHServer.Models
{
    public interface IAggregateRecord
    {
        string Id { get; set; }
        string TimeRange { get; set; }

        ReadingAggregate Temperature { get; set; }
        ReadingAggregate Pressure { get; set; }
        ReadingAggregate Humidity { get; set; }
    }
}
