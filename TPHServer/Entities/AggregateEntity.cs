using AutoMapper;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using TPHServer.Models;

namespace TPHServer.Entities
{
    public class AggregateEntity : TableEntity
    {
        public double TemperatureMinimum { get; set; } = double.MaxValue;
        public double TemperatureMaximum { get; set; } = double.MinValue;
        public double TemperatureTotal { get; set; } = 0;
        public int TemperatureCount { get; set; } = 0;

        public double PressureMinimum { get; set; } = double.MaxValue;
        public double PressureMaximum { get; set; } = double.MinValue;
        public double PressureTotal { get; set; } = 0;
        public int PressureCount { get; set; } = 0;

        public double HumidityMinimum { get; set; } = double.MaxValue;
        public double HumidityMaximum { get; set; } = double.MinValue;
        public double HumidityTotal { get; set; } = 0;
        public int HumidityCount { get; set; } = 0;

        public static IMappingExpression<IAggregateRecord, AggregateEntity> MapToEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<IAggregateRecord, AggregateEntity>()
                .ForMember(dest => dest.PartitionKey, opt => opt.MapFrom(src => src.Id))
                .ForMember(dest => dest.RowKey, opt => opt.MapFrom(src => src.TimeRange))

                .ForMember(dest => dest.TemperatureMinimum, opt => opt.MapFrom(src => src.Temperature.Minimum))
                .ForMember(dest => dest.TemperatureMaximum, opt => opt.MapFrom(src => src.Temperature.Maximum))
                .ForMember(dest => dest.TemperatureTotal, opt => opt.MapFrom(src => src.Temperature.Total))
                .ForMember(dest => dest.TemperatureCount, opt => opt.MapFrom(src => src.Temperature.Count))

                .ForMember(dest => dest.PressureMinimum, opt => opt.MapFrom(src => src.Pressure.Minimum))
                .ForMember(dest => dest.PressureMaximum, opt => opt.MapFrom(src => src.Pressure.Maximum))
                .ForMember(dest => dest.PressureTotal, opt => opt.MapFrom(src => src.Pressure.Total))
                .ForMember(dest => dest.PressureCount, opt => opt.MapFrom(src => src.Pressure.Count))

                .ForMember(dest => dest.HumidityMinimum, opt => opt.MapFrom(src => src.Humidity.Minimum))
                .ForMember(dest => dest.HumidityMaximum, opt => opt.MapFrom(src => src.Humidity.Maximum))
                .ForMember(dest => dest.HumidityTotal, opt => opt.MapFrom(src => src.Humidity.Total))
                .ForMember(dest => dest.HumidityCount, opt => opt.MapFrom(src => src.Humidity.Count))



                ;
        }
        public static IMappingExpression<AggregateEntity, IAggregateRecord> MapFromEntity(IMapperConfigurationExpression c)
        {
            return c.CreateMap<AggregateEntity, IAggregateRecord>()
                .ForPath(dest => dest.Id, opt => opt.MapFrom(src => src.PartitionKey))
                .ForPath(dest => dest.TimeRange, opt => opt.MapFrom(src => src.RowKey))
                .ForPath(dest => dest.Temperature.Minimum, opt => opt.MapFrom(src => src.TemperatureMinimum))
                .ForPath(dest => dest.Temperature.Maximum, opt => opt.MapFrom(src => src.TemperatureMaximum))
                .ForPath(dest => dest.Temperature.Total, opt => opt.MapFrom(src => src.TemperatureTotal))
                .ForPath(dest => dest.Temperature.Count, opt => opt.MapFrom(src => src.TemperatureCount))
                .ForPath(dest => dest.Pressure.Minimum, opt => opt.MapFrom(src => src.PressureMinimum))
                .ForPath(dest => dest.Pressure.Maximum, opt => opt.MapFrom(src => src.PressureMaximum))
                .ForPath(dest => dest.Pressure.Total, opt => opt.MapFrom(src => src.PressureTotal))
                .ForPath(dest => dest.Pressure.Count, opt => opt.MapFrom(src => src.PressureCount))
                .ForPath(dest => dest.Humidity.Minimum, opt => opt.MapFrom(src => src.HumidityMinimum))
                .ForPath(dest => dest.Humidity.Maximum, opt => opt.MapFrom(src => src.HumidityMaximum))
                .ForPath(dest => dest.Humidity.Total, opt => opt.MapFrom(src => src.HumidityTotal))
                .ForPath(dest => dest.Humidity.Count, opt => opt.MapFrom(src => src.HumidityCount))


                ;
        }


    }


}
