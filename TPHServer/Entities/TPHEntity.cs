using AutoMapper;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using TPHServer.Models;

namespace TPHServer.Entities
{
    public class TPHEntity : TableEntity
    {
        public double Humidity { get; set; }
        public double Pressure { get; set; }
        public double Temperature { get; set; }
        public DateTime ReadAt { get; set; }


        public static IMappingExpression<ITPHReading, TPHEntity> MapToEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<ITPHReading, TPHEntity>()
                    .ForMember(dest => dest.RowKey, opt => opt.MapFrom(src => src.ReadAt.ToString("yyMMddHHmmss")))
                    .ForMember(dest => dest.PartitionKey, opt => opt.MapFrom(src => src.Id));

        }
        public static IMappingExpression<TPHEntity, ITPHReading> MapFromEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<TPHEntity, ITPHReading>()
                    //.ForMember(dest => dest.ReadAt, opt => opt.MapFrom(src => src.ReadAt.ToLocalTime()))
                    .ForMember(dest => dest.Id, opt => opt.MapFrom(src => src.PartitionKey));

        }

    }


}
