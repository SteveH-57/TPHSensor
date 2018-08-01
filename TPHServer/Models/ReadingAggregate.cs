using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace TPHServer.Models
{
    public class ReadingAggregate : IReadingAggregate
    {
        public double Minimum { get; set; } = double.MaxValue;
        public double Maximum { get; set; } = double.MinValue;
        public double Total { get; set; } = 0;
        public int Count { get; set; } = 0;
    }
}
