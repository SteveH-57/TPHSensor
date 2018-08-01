namespace TPHServer.Models
{
    public interface IDevice
    {
        string Account { get; set; }
        string DeviceId { get; set; }
        bool Active { get; set; }
        string Description { get; set; }
        string Location { get; set; }
    }
    public class Device : IDevice
    {
        public string Account { get; set;  }
        public string DeviceId { get; set;  }
        public bool Active { get; set;  }
        public string Description { get; set;  }
        public string Location { get; set;  }
    }
}
