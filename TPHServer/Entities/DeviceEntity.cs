using AutoMapper;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using TPHServer.Models;

namespace TPHServer.Entities
{
    public class DeviceEntity : TableEntity
    {
        public bool Active { get; set; }
        public string Description { get; set; }
        public string Location { get; set; }
        public DeviceEntity()
        {

        }
        public static IMappingExpression<IDevice, DeviceEntity> MapToEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<IDevice, DeviceEntity>()
                .ForMember(dest => dest.PartitionKey, opt => opt.MapFrom(src => src.Account))
                .ForMember(dest => dest.RowKey, opt => opt.MapFrom(src => src.DeviceId));

        }
        public static IMappingExpression<DeviceEntity,IDevice> MapFromEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<DeviceEntity, IDevice>()
                .ForMember(dest => dest.Account, opt => opt.MapFrom(src => src.PartitionKey))
                .ForMember(dest => dest.DeviceId, opt => opt.MapFrom(src => src.RowKey));

        }

    }
}
