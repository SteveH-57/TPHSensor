using AutoMapper;
using Microsoft.Extensions.Configuration;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using TPHServer.Entities;
using TPHServer.Models;

namespace TPHServer.Data
{
    public class TPHTableStorageRepo : ITPHRepo
    {
        readonly String connString;
        GenericTableStorage deviceTable;
        GenericTableStorage readingTable;
        GenericTableStorage aggregateTable;
        public TPHTableStorageRepo(IConfiguration configuration)
        {
            connString = configuration["StorageConnectionString"];
            deviceTable = new GenericTableStorage(connString, "Devices");
            readingTable = new GenericTableStorage(connString, "Readings");
            aggregateTable = new GenericTableStorage(connString, "Aggregates");

        }

        public IEnumerable<ITPHReading> GetDeviceReadings(string deviceId)
        {
            var deviceCondition = TableQuery.GenerateFilterCondition("PartitionKey", QueryComparisons.Equal, deviceId);
            var firstCondition = TableQuery.GenerateFilterCondition("RowKey", QueryComparisons.GreaterThanOrEqual, DateTime.Now.AddDays(-7).ToString("yyMMdd")+"000000");
            var lastCondition = TableQuery.GenerateFilterCondition("RowKey", QueryComparisons.LessThanOrEqual,  DateTime.Now.ToString("yyMMdd") + "235959");
            var filter = TableQuery.CombineFilters(deviceCondition, TableOperators.And, firstCondition);
            filter = TableQuery.CombineFilters(filter, TableOperators.And, lastCondition);
            var results = readingTable.GetAllYield<TPHEntity>(filter);
            foreach (var record in results)
            {
                yield return Mapper.Map<TPHEntity, ITPHReading>(record);
            }
        }
        public async Task<IEnumerable<IDevice>> GetAccountDevices(string account)
        {
            var results = await deviceTable.GetAllAsync<DeviceEntity>("PartitionKey", QueryComparisons.Equal, account);
            return Mapper.Map<IEnumerable<DeviceEntity>, IEnumerable<IDevice>>(results);
        }
        public async Task<IEnumerable<IAggregateRecord>> GetAggregates(string deviceId, string type, string first, string last)
        {
            var deviceCondition = TableQuery.GenerateFilterCondition("PartitionKey", QueryComparisons.Equal, deviceId + type.ToUpper());
            var firstCondition = TableQuery.GenerateFilterCondition("RowKey", QueryComparisons.GreaterThanOrEqual, first);
            var lastCondition = TableQuery.GenerateFilterCondition("RowKey", QueryComparisons.LessThanOrEqual, last);
            var filter = TableQuery.CombineFilters(deviceCondition, TableOperators.And, firstCondition);
            filter = TableQuery.CombineFilters(filter, TableOperators.And, lastCondition);
            var results = await aggregateTable.GetAllAsync<AggregateEntity>(filter);
            return Mapper.Map<IEnumerable<AggregateEntity>, IEnumerable<IAggregateRecord>>(results);
        }
        public async Task<ITPHReading> AddReading(ITPHReading reading)
        {
            var result = await readingTable.Insert<TPHEntity>(Mapper.Map<ITPHReading, TPHEntity>(reading));
            if (result.Temperature > -10 && result.Temperature < 110) { await Aggregate(result); }
            return Mapper.Map<TPHEntity, ITPHReading>(result);

        }
        private async Task Aggregate(TPHEntity reading)
        {
            var aggregateTable = new GenericTableStorage(connString, "Aggregates");
            var dayEntityTask = aggregateTable.GetSingle<AggregateEntity>(reading.PartitionKey + "D", reading.ReadAt.ToString("yyMMdd"));
            var hourEntityTask = aggregateTable.GetSingle<AggregateEntity>(reading.PartitionKey + "H", reading.ReadAt.ToString("yyMMddHH"));
            var monthEntityTask = aggregateTable.GetSingle<AggregateEntity>(reading.PartitionKey + "M", reading.ReadAt.ToString("yyMM"));

            var dayEntity = await dayEntityTask;
            if (dayEntity == null) { dayEntity = new AggregateEntity() { PartitionKey = reading.PartitionKey + "D", RowKey = reading.ReadAt.ToString("yyMMdd") }; }
            var dayRecord = Mapper.Map<AggregateEntity, IAggregateRecord>(dayEntity);
            UpdateAggregateValues(reading.Temperature, dayRecord.Temperature);
            UpdateAggregateValues(reading.Pressure, dayRecord.Pressure);
            UpdateAggregateValues(reading.Humidity, dayRecord.Humidity);
            await aggregateTable.InsertOrReplaceAsync(Mapper.Map<IAggregateRecord, AggregateEntity>(dayRecord));

            var hourEntity = await hourEntityTask;
            if (hourEntity == null) { hourEntity = new AggregateEntity() { PartitionKey = reading.PartitionKey + "H", RowKey = reading.ReadAt.ToString("yyMMddHH") }; }
            var hourRecord = Mapper.Map<AggregateEntity, IAggregateRecord>(hourEntity);
            UpdateAggregateValues(reading.Temperature, hourRecord.Temperature);
            UpdateAggregateValues(reading.Pressure, hourRecord.Pressure);
            UpdateAggregateValues(reading.Humidity, hourRecord.Humidity);
            await aggregateTable.InsertOrReplaceAsync(Mapper.Map<IAggregateRecord, AggregateEntity>(hourRecord));

            var monthEntity = await monthEntityTask;
            if (monthEntity == null) { monthEntity = new AggregateEntity() { PartitionKey = reading.PartitionKey + "M", RowKey = reading.ReadAt.ToString("yyMM") }; }
            var monthRecord = Mapper.Map<AggregateEntity, IAggregateRecord>(monthEntity);
            UpdateAggregateValues(reading.Temperature, monthRecord.Temperature);
            UpdateAggregateValues(reading.Pressure, monthRecord.Pressure);
            UpdateAggregateValues(reading.Humidity, monthRecord.Humidity);
            await aggregateTable.InsertOrReplaceAsync(Mapper.Map<IAggregateRecord, AggregateEntity>(monthRecord));




        }
        private void UpdateAggregateValues(double value, IReadingAggregate aggregate)
        {
            aggregate.Count += 1;
            aggregate.Total += value;
            if (aggregate.Minimum > value || aggregate.Minimum == 0)
            {
                aggregate.Minimum = value;
            }
            if (aggregate.Maximum < value || aggregate.Maximum == 0)
            {
                aggregate.Maximum = value;
            }
        }

    }
}
