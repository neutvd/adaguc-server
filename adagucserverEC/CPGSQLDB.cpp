#include "CPGSQLDB.h"
const char *CPGSQLDB::className="CPGSQLDB";
void CPGSQLDB::clearResult(){
  if(result==NULL)PQclear(result);
  result=NULL;
}

const char * CPGSQLDB::getError(){
  return LastErrorMsg;
}


CPGSQLDB::CPGSQLDB(){
  connection = NULL;
  result = NULL;
  dConnected=0;
}

CPGSQLDB::~CPGSQLDB(){
  close();
}

int CPGSQLDB::close(){
  if(dConnected == 1){clearResult();PQfinish(connection);}
  dConnected = 0;
  return 0;
}

int CPGSQLDB::connect(const char * pszOptions){
  LastErrorMsg[0]='\0';
  if(dConnected == 1)return 0;
  connection = PQconnectdb(pszOptions);
  if (PQstatus(connection) == CONNECTION_BAD){
    snprintf(szTemp,CPGSQLDB_MAX_STR_LEN,"Connection to database failed: %s",PQerrorMessage(connection));
    CDBError(szTemp);
    return 1;
  }
  dConnected = 1;
  return 0;
}

int CPGSQLDB::checkTable(const char * pszTableName,const char *pszColumns){
  // returncodes:
  // 0 = no change
  // 1 = error
  // 2 = table created
  
  LastErrorMsg[0]='\0';
  int i;
  if(dConnected == 0){
    CDBError("checkTable: Not connected to DB");
    return 1;
  }
  char query_string[256];
  snprintf(query_string,255,"select * from pg_tables where schemaname='public';");
  result = PQexec(connection, query_string);                   /* send the query */
  if (PQresultStatus(result) != PGRES_TUPLES_OK)         /* did the query fail? */
  {
    CDBError("checkTable: select from pg_tables failed");
    clearResult();
    return 1;
  }

  for (i = 0; i < PQntuples(result); i++){
    char *pqval=PQgetvalue(result, i, 1);
    if(strncmp(pszTableName,pqval,strlen(pszTableName))==0&&
       strlen(pqval)==strlen(pszTableName))break;
  }
  // No table exists yet
  if(i == PQntuples(result)){
    clearResult();
    snprintf(query_string,255,"CREATE TABLE %s (%s)",pszTableName,pszColumns);
    result = PQexec(connection, query_string);                   /* send the query */
    if (PQresultStatus(result) != PGRES_COMMAND_OK)         /* did the query fail? */
    {
      //snprintf(szTemp,CPGSQLDB_MAX_STR_LEN,"checkTable: CREATE TABLE %s failed",pszTableName);
      //CDBError(szTemp);
      clearResult();
      return 1;
    }
    //Table created set status to 2
    clearResult();
    return 2;
  }
  clearResult();
  return 0;
}

int CPGSQLDB::query(const char *pszQuery){
  LastErrorMsg[0]='\0';
  if(dConnected == 0){
    CDBError("query: Not connected to DB");
    return 1;
  }
  result = PQexec(connection, pszQuery);
  if (PQresultStatus(result) != PGRES_COMMAND_OK)         /* did the query fail? */
  {
    //snprintf(szTemp,CPGSQLDB_MAX_STR_LEN,"query: [%s] failed: %s",pszQuery,PQerrorMessage(connection));
    //CDBError(szTemp);
    clearResult();
    return 1;
  }
  clearResult();
  return 0;
}
CT::string* CPGSQLDB::query_select(const char *pszQuery,int dColumn){
  LastErrorMsg[0]='\0';
  int i;
  if(dConnected == 0){
    CDBError("query_select: Not connected to DB");
    return NULL;
  }

  result = PQexec(connection, pszQuery);

  if (PQresultStatus(result) != PGRES_TUPLES_OK) // did the query fail? 
  {
    //snprintf(szTemp,CPGSQLDB_MAX_STR_LEN,"query_select: %s failed",pszQuery);
    //CDBError(szTemp);
    clearResult();
    return NULL;
  }
  int n=PQntuples(result);
  CT::string *strings=new CT::string[n+1];
  for(i=0;i<n;i++){
    strings[i].copy(PQgetvalue(result, i, dColumn));
    strings[i].count=n;
  }
  CT::CTlink<CT::string>(strings,n);
  clearResult();
  return strings;
}
CT::string* CPGSQLDB::query_select(const char *pszQuery){
  return query_select(pszQuery,0);
}


CDB::Store* CPGSQLDB::queryToStore(const char *pszQuery){
  LastErrorMsg[0]='\0';

  if(dConnected == 0){
    CDBError("queryToStore: Not connected to DB");
    return NULL;
  }

  result = PQexec(connection, pszQuery);

  if (PQresultStatus(result) != PGRES_TUPLES_OK) // did the query fail? 
  {
    //snprintf(szTemp,CPGSQLDB_MAX_STR_LEN,"query_select: %s failed",pszQuery);
    //CDBError(szTemp);
    clearResult();
    return NULL;
  }
  size_t numCols=PQnfields(result);
  size_t numRows=PQntuples(result);
  if(numCols==0){
    clearResult();
    return NULL;
  }
  ColumnModel *colModel = new ColumnModel(PQnfields(result));
 // colModel 
  
  
  for(size_t colNumber=0;colNumber<numCols;colNumber++){
    colModel->setColumn(colNumber,PQfname(result,colNumber));
  }
  
  Store *store=new Store(colModel);
  
  
  
  
  for(size_t rowNumber=0;rowNumber<numRows;rowNumber++){
    Record *record = new Record(colModel);
    for(size_t colNumber=0;colNumber<numCols;colNumber++)record->push(colNumber,PQgetvalue(result, rowNumber, colNumber));
    store->push(record);
  }
 
  clearResult();
  return store;;
}