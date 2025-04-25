#include <utility>
#include <list>
#include <algorithm>
#include "hash.hpp"
#include <iostream>

HashSet::Iterator HashSet::begin() {
  return master_list.begin();
}

HashSet::Iterator HashSet::end() {
  return master_list.end();
}

HashSet::HashSet() {
  load_factor = num_element = current_size_index = 0;
  max_load_factor = 1;
  master_list = std::list<int>();
  hash_set.resize(sizes.at(current_size_index));
  std::fill(hash_set.begin(), hash_set.end(), master_list.end());
}

HashSet::HashSet(const HashSet& other) {
  load_factor = other.load_factor;
  max_load_factor = other.max_load_factor;
  num_element = other.num_element; 
  current_size_index = other.current_size_index;
  hash_set.resize(sizes.at(current_size_index));
  std::fill(hash_set.begin(), hash_set.end(), master_list.end());

  for (int node : other.master_list) {
    master_list.push_back(node);
  }

  for (std::list<int>::iterator it = master_list.begin(); it != master_list.end(); it++) {
    std::size_t bucket_index = bucket(*it);

    if (hash_set.at(bucket_index) == master_list.end()) { // check if bucket has a starting point yet
      hash_set.at(bucket_index) = it;    
    }
  }
}

HashSet& HashSet::operator=(HashSet other) {
  std::swap(num_element, other.num_element);
  std::swap(current_size_index, other.current_size_index);
  std::swap(load_factor, other.load_factor);
  std::swap(max_load_factor, other.max_load_factor);
  std::swap(hash_set, other.hash_set);
  std::swap(master_list, other.master_list);
  return *this;
}

HashSet::~HashSet() {
}

void HashSet::insert(int key) {
  if (load_factor >= max_load_factor) {
    rehash(sizes.at(++current_size_index));
  }

  std::size_t bucket_index = bucket(key);

  if (master_list.empty()) {
    master_list.push_back(key);
    hash_set.at(bucket_index) = master_list.begin();
  } else {
    if (contains(key)) {
      return;
    } 

    //insert after iterator pointing to first element in the list that hashes to bucket_index
    if (hash_set.at(bucket_index) != master_list.end()) {
      std::list<int>::iterator it = hash_set.at(bucket_index);
      advance(it, 1);
      master_list.insert(it, key);
    } else {
      //first instance of an element hashes to bucket_index = push_back in master_list
      master_list.push_back(key);
      std::list<int>::iterator it = master_list.end();
      advance(it, -1);
      hash_set.at(bucket_index) = it;
    }
  }

  num_element++;
  load_factor = static_cast<float>(num_element) / sizes.at(current_size_index);
}

bool HashSet::contains(int key) const {
  std::size_t bucket_index = bucket(key);

  if (hash_set.at(bucket_index) == master_list.end()) {
    return false;
  } else {
    std::list<int>::iterator it = hash_set.at(bucket_index);

    //didn't use std::find() to avoid unecessary searching after certain that the key doesn't exist
    do {
      if (*it == key) {
        return true;
      } else {
        advance(it, 1);
      }
    } while(it != master_list.end() && bucket(*it) == bucket_index);
  
    return false;
  }
}

HashSet::Iterator HashSet::find(int key) {
  std::size_t bucket_index = bucket(key);

  if (hash_set.at(bucket_index) == master_list.end()) {
    return master_list.end();
  } else {
    std::list<int>::iterator it = hash_set.at(bucket_index);

    //didn't use std::find() to avoid unecessary searching after certain the key doesn't exist
    do {
      if (*it == key) {
        return it;
      } else {
        advance(it, 1);
      }
    } while(it != master_list.end() && bucket(*it) == bucket_index);

    return master_list.end();
  }
}

void HashSet::erase(int key) {
  std::size_t bucket_index = bucket(key);
  std::list<int>::iterator it = find(key);
  std::list<int>::iterator next = std::next(it);

  if (it == master_list.end()) {
    return;
  }

  if (it == hash_set.at(bucket_index)) { //Check if we are deleting the head of a bucket
    if (bucket(*next) == bucket_index) {
      hash_set.at(bucket_index) = next;
    } else {
      hash_set.at(bucket_index) = master_list.end();
    }
  }

  master_list.erase(it);
  num_element--;
  load_factor = static_cast<float>(num_element) / sizes.at(current_size_index);
}

HashSet::Iterator HashSet::erase(HashSet::Iterator it) {
  std::list<int>::iterator next = std::next(it);
  erase(*it);
  return next;
}

void HashSet::rehash(std::size_t newSize) {
  hash_set.resize(newSize);
  std::fill(hash_set.begin(), hash_set.end(), master_list.end()); // reset the hash_set since rehash will invalidate all nodes position
  std::list<int>::iterator it = master_list.begin();
  
  while (it != master_list.end()) {
    std::list<int>::iterator next = std::next(it); //keep track of next node for next iteration after potentially moving the current node
    std::size_t bucket_index = bucket(*it);

    if (hash_set.at(bucket_index) == master_list.end()) { // check if bucket_index has a starting point
      hash_set.at(bucket_index) = it;
    } else {
      std::list<int>::iterator insert_pos = std::next(hash_set.at(bucket_index)); //insert after iterator pointing to first element in the list that hashes to bucket_index
      master_list.splice(insert_pos, master_list, it);
      it = next;
    }
  }

  load_factor = static_cast<float>(num_element) / newSize;
}

std::size_t HashSet::size() const {
  return num_element;
}

bool HashSet::empty() const {
  return num_element == 0;
}

std::size_t HashSet::bucketCount() const {
  return sizes.at(current_size_index);
}

std::size_t HashSet::bucketSize(std::size_t b) const {
  std::size_t size = 0;
  std::list<int>::iterator it = hash_set.at(b);

  if (it == master_list.end()) {
    return size;
  } else {
    do {
      if (bucket(*it) == b) {
        size++;
        advance(it, 1);
      }
    } while(it != master_list.end() && bucket(*it) == b);
    
    return size;
  }
}

std::size_t HashSet::bucket(int key) const {
  int bucket_index = key % static_cast<int>(sizes.at(current_size_index)); //Negative key will give negative bucket index

  if (bucket_index < 0) { //Fix negative bucket index
    bucket_index *= -1;
  } 

  return bucket_index;
}

float HashSet::loadFactor() const {
  return load_factor;
}

float HashSet::maxLoadFactor() const {
  return max_load_factor;
}

void HashSet::maxLoadFactor(float maxLoad) {
  max_load_factor = maxLoad;

  while (load_factor >= max_load_factor) {
    rehash(sizes.at(++current_size_index));
  }
}

void HashSet::print() { //delete when finish debugging
  for (int node : master_list) {
    std::cout << node << "            " << bucket(node) << '\n';
  }
}
