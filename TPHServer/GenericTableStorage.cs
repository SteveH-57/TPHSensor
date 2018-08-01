using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace TPHServer
{
    //R&D https://docs.microsoft.com/en-us/azure/visual-studio/vs-storage-aspnet5-getting-started-tables
    public class GenericTableStorage
    {
        CloudTable Table;

        public GenericTableStorage(CloudStorageAccount storageAccount, string tableName)
        {
            // Create the table client.
            CloudTableClient tableClient = storageAccount.CreateCloudTableClient();

            // Create the table if it doesn't exist.
            Table = tableClient.GetTableReference(tableName);
            var created = Table.CreateIfNotExistsAsync().Result;
        }
        public GenericTableStorage(string connectionString, string tableName) : this(CloudStorageAccount.Parse(connectionString), tableName)
        {
        }

        public async Task<TEntity> Insert<TEntity>(TEntity data) where TEntity : TableEntity, new()
        {
            var insertOperation = TableOperation.Insert(data);
            var result = await Table.ExecuteAsync(insertOperation);
            return (TEntity) result.Result;
        }

        public async Task InsertOrReplaceAsync<TEntity>(TEntity entity) where TEntity : TableEntity, new()
        {
            if (entity == null) throw new ArgumentNullException("entity");
            entity.ETag = "*";
            var operation = TableOperation.InsertOrReplace(entity);
            await Table.ExecuteAsync(operation);
        }
        public async Task<IEnumerable<TEntity>> GetAllAsync<TEntity>(string field, string condition, string value) where TEntity : TableEntity, new()
        {
            if (String.IsNullOrWhiteSpace(field)) throw new ArgumentNullException("field name");
            try
            {
                var filter = TableQuery.GenerateFilterCondition(field, condition, value);
                return await GetAllAsync<TEntity>(filter);

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                throw;
            }
        }
        public async Task<IEnumerable<TEntity>> GetAllAsync<TEntity>(string filter) where TEntity : TableEntity, new()
        {
            TableContinuationToken token = null;
            var results = new List<TEntity>();

            if (String.IsNullOrWhiteSpace(filter)) throw new ArgumentNullException("filter");
            try
            {
                var query = new TableQuery<TEntity>().Where(filter);

                do
                {
                    var segments = await Table.ExecuteQuerySegmentedAsync(query, token);
                    token = segments.ContinuationToken;
                    results.AddRange(segments.Results);
                } while (token != null);

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                throw;
            }
            return results;
        }
        public IEnumerable<TEntity> GetAllYield<TEntity>(string field, string condition, string value) where TEntity : TableEntity, new()
        {
            return GetAllYield<TEntity>(TableQuery.GenerateFilterCondition(field, condition, value));
        }
        public IEnumerable<TEntity> GetAllYield<TEntity>(string filter) where TEntity : TableEntity, new()
        {
            TableContinuationToken token = null;
            var results = new List<TEntity>();

            var query =
                new TableQuery<TEntity>().Where(filter);
            var i = 0;
            do
            {
                var segments = Table.ExecuteQuerySegmentedAsync(query, token).Result;
                token = segments.ContinuationToken;
                foreach (var record in segments.Results)
                {
                    yield return record;
                }
            } while (token != null && i++ < 2);
        }

        public async Task<TEntity> GetSingle<TEntity>(string partitionKey, string rowKey) where TEntity : TableEntity, new()
        {
            var retrieveOperation = TableOperation.Retrieve<TEntity>(partitionKey, rowKey);
            TableResult retrievedResult = await Table.ExecuteAsync(retrieveOperation);

            if (retrievedResult.Result != null)
            {
                return retrievedResult.Result as TEntity;
            }
            return null;
        }
        public async Task<bool> Delete<TEntity>(string partitionKey, string rowKey) where TEntity : TableEntity, new()
        {
            var retrieveOperation = TableOperation.Retrieve<TEntity>(partitionKey, rowKey);
            TableResult retrievedResult = await Table.ExecuteAsync(retrieveOperation);

            var  deleteEntity = (TEntity)retrievedResult.Result;

            if (deleteEntity != null)
            {
                var deleteOperation = TableOperation.Delete(deleteEntity);
                await Table.ExecuteAsync(deleteOperation);
                return true;
            }

            else
            {
                return false;
            }
        }
    }
   }
