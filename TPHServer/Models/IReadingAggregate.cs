using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace TPHServer.Models
{
    public interface IReadingAggregate
    {
        double Minimum { get; set; }
        double Maximum { get; set; }
        double Total { get; set; }
        int Count { get; set; }
    }
}
