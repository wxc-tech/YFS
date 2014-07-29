// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
  assert(pthread_mutex_init(&count_mutex,NULL) == 0);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  pthread_mutex_lock(&count_mutex);
  std::map<unsigned int, pthread_cond_t>::iterator cond_map_pair;
  std::map<unsigned int,unsigned int>::iterator lock_exist;
  lock_protocol::status ret = lock_protocol::OK;
  pthread_cond_t temp_cond;

  cond_map_pair = cond_map.find(lid);
  if(cond_map_pair == cond_map.end())
  {
    pthread_cond_init(&cond_map[lid], 0);
  }

  printf("lock %d acquire request from clt %d\n",lid,clt);
  r = nacquire;
  lock_exist = lock_map.find(lid);
  if(lock_exist != lock_map.end())//find
  {
    if(lock_exist->second == 1)//has been occupied
    {
      pthread_cond_wait(&cond_map[lid],&count_mutex);
      lock_exist->second = 1;
    }
    else//not been occupied
      lock_exist->second = 1;
  }
  else//not find
    lock_map[lid] = 1;

  pthread_mutex_unlock(&count_mutex);
  printf("lock %d has been grant to clt %d\n",lid,clt);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  pthread_mutex_lock(&count_mutex);
  printf("lock %d release request from clt %d\n",lid,clt);
  std::map<unsigned int,unsigned int>::iterator lock_exist;
  lock_protocol::status ret = lock_protocol::OK;
  std::map<unsigned int, pthread_cond_t>::iterator cond_map_pair;
  r = nacquire;
  cond_map_pair = cond_map.find(lid);
  lock_exist = lock_map.find(lid);
  if(lock_exist != lock_map.end())
  {
    lock_exist->second = 0;
    pthread_cond_signal(&(cond_map_pair->second));
  }
  printf("lock %d has been released according to clt %d\n",lid,clt);
  pthread_mutex_unlock(&count_mutex);
  return ret;
}

