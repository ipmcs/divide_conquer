#include "ooo_cpu.h"
#include <bits/stdc++.h>
using namespace std;

//#define BTB_prefill_on_cache_fill
#define BTB_prefill_on_prefetch_hit_and_cache_fill

string branch_name(uint8_t bb_type) {
  if (bb_type == 0) return "NOTB"; //"NOT_BRANCH";
  if (bb_type == 1) return "DJMP";//"BRANCH_DIRECT_JUMP";
  if (bb_type == 2) return "INDR"; //"BRANCH_INDIRECT";
  if (bb_type == 3) return "COND"; //"BRANCH_CONDITIONAL";
  if (bb_type == 4) return "DCLL"; //"BRANCH_DIRECT_CALL";
  if (bb_type == 5) return "ICLL"; //"BRANCH_INDIRECT_CALL";
  if (bb_type == 6) return "RETN"; //"BRANCH_RETURN";
  if (bb_type == 7) return "OTHR"; //"BRANCH_OTHER";
  if (bb_type == 8) return "EXCB"; //"BRANCH_EXC";
  return "NOT_FOUND\n";
}


class PC_BTB_ENTRY{
public:
  uint64_t tag = 0;
  uint64_t target = 0;
  uint8_t branch_type = 0;
  bool was_taken = 0;

  PC_BTB_ENTRY(uint64_t tg = 0, uint64_t trg = 0, uint8_t bt = 0){
    tag = tg;
    target = trg;
    branch_type = bt;
  }
};

map<uint64_t, PC_BTB_ENTRY> perfect_PC_BTB;

class PC_BTB{
  public:
    uint64_t num_of_sets;
    uint64_t num_of_ways;

    uint64_t num_of_sets_bits;

    uint64_t num_of_lookups = 0;
    uint64_t num_of_updates = 0;
    uint64_t num_of_predecodes = 0;

    uint64_t num_of_hits = 0;
    uint64_t num_of_misses  = 0;

    uint64_t num_of_hits_demand = 0;
    uint64_t num_of_misses_demand  = 0;

    uint64_t num_of_hits_prefill = 0;
    uint64_t num_of_misses_prefill  = 0;

    uint64_t num_of_hits_wrong_path = 0;
    uint64_t num_of_misses_wrong_path = 0;

    uint64_t btb_miss_stalls = 0;
    uint64_t btb_miss_not_stalls = 0;

    uint64_t branch_direction_stalls = 0;
    uint64_t target_prediction_stalls = 0;

    uint64_t direction_mispredictions_do_not_stall = 0;

    uint64_t hash_domain = 1 << 12;
    uint64_t num_of_hash_conflicts = 0;

    uint64_t last_block_lookup = 0;
    uint64_t num_of_block_lookups = 0;

    vector< vector<PC_BTB_ENTRY> > table;
    vector< uint64_t> RAS;

    uint64_t target_misprediction_reason[9] = {0};
    uint64_t target_match_reason[9] = {0};

    bool prefill_enable;
    string name;


    PC_BTB(uint64_t s, uint64_t w, bool pe, string n){
      prefill_enable = pe;

      name = n;

      num_of_sets = s;
      num_of_ways = w;

      num_of_sets_bits = log2(num_of_sets);

      table.resize(num_of_sets);
      for(uint i = 0; i < num_of_sets; i++){
        table[i].resize(num_of_ways);
      }
    }

    uint64_t flookup(uint64_t addr, bool isBranch){
      num_of_lookups++;
      if((addr >> 6) != last_block_lookup){
        num_of_block_lookups++;
        last_block_lookup = addr >> 6;
      }
      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag != t && table[s][i].tag % hash_domain == t % hash_domain){
          num_of_hash_conflicts++;
          break;
        }
      }

      if(!isBranch){
        return 0;
      }

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          num_of_hits++;
          num_of_hits_wrong_path++;

          uint64_t target = table[s][i].target;

          PC_BTB_ENTRY tmp = table[s][i];
          table[s].erase(table[s].begin() + i);
          table[s].push_back(tmp);

          return target;
        }
      }

      num_of_misses++;
      num_of_misses_wrong_path++;

      return addr + 4;
    }

    PC_BTB_ENTRY get_entry(uint64_t addr){
      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          return table[s][i];
        }
      }

      return PC_BTB_ENTRY();
    }


    pair<int, uint64_t> lookup(uint64_t addr, bool prefill_lookup = false, uint8_t direction = 0, uint8_t pDirection = 0, uint64_t target = 0, uint8_t branch_type = 0){
      pair<int, uint64_t> ret_val = make_pair(-1, 0);
      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      if(!prefill_lookup){
        num_of_lookups++;
        if((addr >> 6) != last_block_lookup){
          num_of_block_lookups++;
          last_block_lookup = addr >> 6;
        }

        for(uint i = 0; i < num_of_ways; i++){
          if(table[s][i].tag != t && table[s][i].tag % hash_domain == t % hash_domain){
            num_of_hash_conflicts++;
            break;
          }
        }
      }

      if(branch_type == NOT_BRANCH){
        return ret_val;
      } 

      uint64_t pTarget = addr + 4;

      if(!prefill_lookup){
        if(branch_type == BRANCH_INDIRECT_CALL || branch_type == BRANCH_DIRECT_CALL){
          RAS.push_back(addr + 4);
        }
        else if(branch_type == BRANCH_RETURN && !RAS.empty()){
          pTarget = RAS.back();
          RAS.erase(RAS.begin() + RAS.size() - 1);
        }
      }

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          num_of_hits++;
          if(prefill_lookup){
            num_of_hits_prefill++;
          }
          else{
            num_of_hits_demand++;
            if(branch_type == BRANCH_RETURN && !RAS.empty() && pDirection){
            }
            else{
              if(pDirection){
                pTarget = table[s][i].target;
              }
              else{
                pTarget = addr + 4;
              }
            }

            if(direction == pDirection){ 
              if(pTarget == target){
                target_match_reason[branch_type]++;
                ret_val = make_pair(0, pTarget);
              }
              else{
                ret_val = make_pair(6, pTarget);
                target_misprediction_reason[branch_type]++;
                target_prediction_stalls++;
              }
            }
            else{
              if(!direction && pDirection && table[s][i].target == (addr + 4)){
                direction_mispredictions_do_not_stall++;
                ret_val = make_pair(0, pTarget);
              }
              else{
                ret_val = make_pair(4, pTarget);
                branch_direction_stalls++;
              }
            }
          }

          if(prefill_lookup){
            num_of_updates++;
          }
          else{
            if(direction){
              if(pTarget == target){
                return ret_val;
              }
              else{
                if(pTarget != target){
                  if(branch_type != BRANCH_RETURN){
                    num_of_updates++;
                  }
                  else{
                    if(pTarget == (addr + 4)){
                      num_of_updates++;
                    }
                    else{
                      return ret_val;
                    }
                  }
                }
                else{
                  return ret_val;
                }
              }
            }
            else{
              return ret_val;
            }
          }

          PC_BTB_ENTRY tmp = table[s][i];
          if(direction){
            tmp.target = target;
          }
          table[s].erase(table[s].begin()+i);
          table[s].push_back(tmp);

          if(!prefill_lookup && ret_val.first == -1){
            assert(false);
          }
          return ret_val;
        }
      }

      num_of_misses++;
      if(prefill_lookup){
        num_of_misses_prefill++;

        num_of_updates++;

        // prefilling mechanism may insert all branches!
        table[s].erase(table[s].begin());
        table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));
      }
      else{
        num_of_misses_demand++;
        if(direction){
          btb_miss_stalls++;

          num_of_updates++;

          table[s].erase(table[s].begin());
          table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));
          ret_val = make_pair(1, addr+4);
        }
        else{
          btb_miss_not_stalls++;

          // Our BTB inserts an entry if it finds the missing branch is taken. 
          // If you want to insert also not taken branches, uncomment the following lines
          //num_of_updates++;
          //table[s].erase(table[s].begin());
          //table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));

          ret_val = make_pair(0, addr+4);
        }
      }

      return ret_val;
    }

    void prefill(uint64_t addr){
      if(!prefill_enable){
        return;
      }

      num_of_predecodes++; 
      addr = ((addr >> 6) << 6);
      for(uint64_t i = addr; i < (addr + 64); i += 4){
        if(perfect_PC_BTB.find(i) != perfect_PC_BTB.end()){
          if(perfect_PC_BTB[i].branch_type == BRANCH_INDIRECT || perfect_PC_BTB[i].branch_type == BRANCH_INDIRECT_CALL){
            continue;
          }

          lookup(i, true, false, false, perfect_PC_BTB[i].target, perfect_PC_BTB[i].branch_type);
        }
      }
    }  

    void printStat(){
      cout << "SHEET\n";
      cout << name << ", num_of_lookups: " << num_of_lookups << "\n";
      cout << name << ", num_of_updates: " << num_of_updates << "\n";
      cout << name << ", num_of_predecodes: " << num_of_predecodes << "\n";
      cout << name << ", num_of_hits: " << num_of_hits << "\n";
      cout << name << ", num_of_misses: " << num_of_misses << "\n";
      cout << name << ", hit rate: " << (double)num_of_hits/(num_of_hits+num_of_misses) << "\n";
      cout << name << ", num_of_hits_demand: " << num_of_hits_demand << "\n";
      cout << name << ", num_of_misses_demand: " << num_of_misses_demand << "\n";
      cout << name << ", hit rate demand: " << (double)num_of_hits_demand/(num_of_hits_demand+num_of_misses_demand) << "\n";
      cout << name << ", num_of_hits_prefill: " << num_of_hits_prefill << "\n";
      cout << name << ", num_of_misses_prefill: " << num_of_misses_prefill << "\n";
      cout << name << ", hit rate prefill: " << (double)num_of_hits_prefill/(num_of_hits_prefill+num_of_misses_prefill) << "\n";
      cout << name << ", num_of_hits_wrong_path: " << num_of_hits_wrong_path << "\n";
      cout << name << ", num_of_misses_wrong_path: " << num_of_misses_wrong_path << "\n";
      cout << name << ", btb_miss_stalls: " << btb_miss_stalls << "\n";
      cout << name << ", btb_miss_not_stalls: " << btb_miss_not_stalls << "\n";
      cout << name << ", direction_mispredictions_do_not_stall: " << direction_mispredictions_do_not_stall << "\n";   
      cout << name << ", branch_direction_stalls: " << branch_direction_stalls << "\n";
      cout << name << ", target_prediction_stalls: " << target_prediction_stalls << "\n";
      for(int i = 0; i < 9; i++){
        cout << name << ", target misprediction reason " << branch_name(i) << ": " << target_misprediction_reason[i] << "\n";
      }
      for(int i = 0; i < 9; i++){
        cout << name << ", target match reason " << branch_name(i) << ": " << target_match_reason[i] << "\n";
      }
      cout << name << ", num_of_hash_conflicts: " << num_of_hash_conflicts << "\n";
      cout << name << ", num_of_block_lookups: " << num_of_block_lookups << "\n";

      cout << "\n\n\n"; 
    }

    void printStalls(O3_CPU *core){
      cout << name << " btb_miss_stalls: " << (1000.0*btb_miss_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " branch_direction_stalls: " << (1000.0*branch_direction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " target_prediction_stalls: " << (1000.0*target_prediction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
    }


    void reset(){
      num_of_lookups = 0;
      num_of_updates = 0;
      num_of_predecodes = 0;

      num_of_hits = 0;
      num_of_misses  = 0;

      num_of_hits_demand = 0;
      num_of_misses_demand  = 0;

      num_of_hits_prefill = 0;
      num_of_misses_prefill  = 0;

      btb_miss_stalls = 0;
      btb_miss_not_stalls = 0;

      branch_direction_stalls = 0;
      target_prediction_stalls = 0;

      direction_mispredictions_do_not_stall = 0;

      for(int i = 0; i < 9; i++){
        target_misprediction_reason[i] = 0;
        target_match_reason[i] = 0;
      }

      num_of_block_lookups = 0;
      num_of_hash_conflicts = 0;
    }

};


class Confluence_BTB{
  public:
    struct Confluence_BTB_entry{
      uint64_t tag;
      vector<PC_BTB_ENTRY> branches;
    };

    uint64_t num_of_sets;
    uint64_t num_of_ways;

    uint64_t num_of_sets_bits;

    uint64_t num_of_lookups = 0;
    uint64_t num_of_updates = 0;
    uint64_t num_of_predecodes = 0;

    uint64_t num_of_hits = 0;
    uint64_t num_of_misses  = 0;

    uint64_t num_of_hits_demand = 0;
    uint64_t num_of_misses_demand  = 0;

    uint64_t num_of_hits_prefill = 0;
    uint64_t num_of_misses_prefill  = 0;

    uint64_t num_of_hits_wrong_path = 0;
    uint64_t num_of_misses_wrong_path = 0;

    uint64_t btb_miss_stalls = 0;
    uint64_t btb_miss_not_stalls = 0;

    uint64_t branch_direction_stalls = 0;
    uint64_t target_prediction_stalls = 0;

    uint64_t direction_mispredictions_do_not_stall = 0;

    vector< vector<Confluence_BTB_entry> > table;
    vector<PC_BTB_ENTRY> overflow_buffer;
    vector< uint64_t> RAS;

    uint64_t target_misprediction_reason[9] = {0};
    uint64_t target_match_reason[9] = {0};

    bool prefill_enable;
    string name;

    uint64_t last_lookup_block;

    uint32_t bundle_size = 3;

    Confluence_BTB(uint64_t s, uint64_t w, bool pe, string n){
      prefill_enable = pe;

      name = n;

      num_of_sets = s;
      num_of_ways = w;

      num_of_sets_bits = log2(num_of_sets);

      table.resize(num_of_sets);
      for(uint i = 0; i < num_of_sets; i++){
        table[i].resize(num_of_ways);
      }

      overflow_buffer.resize(32);
    }

    uint64_t flookup(uint64_t addr){
      if((addr >> LOG2_BLOCK_SIZE) != last_lookup_block){
        num_of_lookups++;
        last_lookup_block = (addr >> LOG2_BLOCK_SIZE);
      }


      if(perfect_PC_BTB.find(addr) == perfect_PC_BTB.end()){
        return addr + 4;
      }

      uint64_t s = (addr >> 6) % num_of_sets;
      uint64_t t = (addr >> 6) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          for(uint j = 0; j < table[s][i].branches.size(); j++){
            if(table[s][i].branches[j].tag == addr){
              num_of_hits++;
              num_of_hits_wrong_path++;

              uint64_t target = table[s][i].branches[j].target;
              return target;
            }
          }
        }
      }

      for(uint i = 0; i < overflow_buffer.size(); i++){
        if(overflow_buffer[i].tag == addr){
          num_of_hits++;
          num_of_hits_wrong_path++;
          
          return overflow_buffer[i].target;
        }
      }

      num_of_misses++;
      num_of_misses_wrong_path++;
      return addr + 4;
    }

    PC_BTB_ENTRY get_entry(uint64_t addr){
      uint64_t s = (addr >> 6) % num_of_sets;
      uint64_t t = (addr >> 6) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          for(uint j = 0; j < table[s][i].branches.size(); j++){
            if(table[s][i].branches[j].tag == addr){
              return table[s][i].branches[j];
            }
          }
        }
      }

      return PC_BTB_ENTRY();
    }


    pair<int, uint64_t> lookup(uint64_t addr, bool prefill_lookup = false, uint8_t direction = 0, uint8_t pDirection = 0, uint64_t target = 0, uint8_t branch_type = 0){
      if(prefill_lookup){ assert(false); }

      pair<int, uint64_t> ret_val = make_pair(-1, 0);

      if(prefill_lookup){
        assert(false);
      }

      if((addr >> LOG2_BLOCK_SIZE) != last_lookup_block){
        num_of_lookups++;
        last_lookup_block = (addr >> LOG2_BLOCK_SIZE);
      }


      if(branch_type == NOT_BRANCH){
        return ret_val;
      } 

      uint64_t pTarget = addr + 4;

      if(!prefill_lookup){
        if(branch_type == BRANCH_INDIRECT_CALL || branch_type == BRANCH_DIRECT_CALL){
          RAS.push_back(addr + 4);
        }
        else if(branch_type == BRANCH_RETURN && !RAS.empty()){
          pTarget = RAS.back();
          RAS.erase(RAS.begin() + RAS.size() - 1);
        }
      }


      uint64_t s = (addr >> 6) % num_of_sets;
      uint64_t t = (addr >> 6) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          bool found = false;
          for(uint j = 0; j < table[s][i].branches.size(); j++){
            if(table[s][i].branches[j].tag == addr){
              found = true;
              num_of_hits++;
              if(prefill_lookup){
                num_of_hits_prefill++;
              }
              else{
                num_of_hits_demand++;
                if(branch_type == BRANCH_RETURN && !RAS.empty() && pDirection){
                }
                else{
                  if(pDirection){
                    pTarget = table[s][i].branches[j].target;
                  }
                  else{
                    pTarget = addr + 4;
                  }
                }

                if(direction == pDirection){ 
                  if(pTarget == target){
                    target_match_reason[branch_type]++;
                    ret_val = make_pair(0, pTarget);
                  }
                  else{
                    ret_val = make_pair(6, pTarget);
                    target_misprediction_reason[branch_type]++;
                    target_prediction_stalls++;
                  }
                }
                else{
                  if(!direction && pDirection && table[s][i].branches[j].target == (addr + 4)){
                    direction_mispredictions_do_not_stall++;
                    ret_val = make_pair(0, pTarget);
                  }
                  else{
                    ret_val = make_pair(4, pTarget);
                    branch_direction_stalls++;
                  }
                }
              }

              if(prefill_lookup){
                num_of_updates++;
              }
              else{
                if(direction){
                  if(pTarget == target){
                    return ret_val;
                  }
                  else{
                    if(pTarget != target){
                      if(branch_type != BRANCH_RETURN){
                        num_of_updates++;
                      }
                      else{
                        if(pTarget == (addr + 4)){
                          num_of_updates++;
                        }
                        else{
                          return ret_val;
                        }
                      }
                    }
                    else{
                      return ret_val;
                    }
                  }
                }
                else{
                  return ret_val;
                }
              }

              if(direction){
                table[s][i].branches[j].target = target;
              }

              if(!prefill_lookup && ret_val.first == -1){
                assert(false);
              }
              return ret_val;
            } 
          }        
          if(!found){
            for(uint idx = 0; idx < overflow_buffer.size(); idx++){
              if(overflow_buffer[idx].tag == addr){
                found = true;
                num_of_hits++;
                if(prefill_lookup){
                  num_of_hits_prefill++;
                }
                else{
                  num_of_hits_demand++;
                  if(branch_type == BRANCH_RETURN && !RAS.empty() && pDirection){
                  }
                  else{
                    if(pDirection){
                      pTarget = overflow_buffer[idx].target;
                    }
                    else{
                      pTarget = addr + 4;
                    }
                  }

                  if(direction == pDirection){ 
                    if(pTarget == target){
                      target_match_reason[branch_type]++;
                      ret_val = make_pair(0, pTarget);
                    }
                    else{
                      ret_val = make_pair(6, pTarget);
                      target_misprediction_reason[branch_type]++;
                      target_prediction_stalls++;
                    }
                  }
                  else{
                    if(!direction && pDirection && overflow_buffer[idx].target == (addr + 4)){
                      direction_mispredictions_do_not_stall++;
                      ret_val = make_pair(0, pTarget);
                    }
                    else{
                      ret_val = make_pair(4, pTarget);
                      branch_direction_stalls++;
                    }
                  }
                }

                if(prefill_lookup){
                  num_of_updates++;
                }
                else{
                  if(direction){
                    if(pTarget == target){
                      return ret_val;
                    }
                    else{
                      if(pTarget != target){
                        if(branch_type != BRANCH_RETURN){
                          num_of_updates++;
                        }
                        else{
                          if(pTarget == (addr + 4)){
                            num_of_updates++;
                          }
                          else{
                            return ret_val;
                          }
                        }
                      }
                      else{
                        return ret_val;
                      }
                    }
                  }
                  else{
                    return ret_val;
                  }
                }

                if(direction){
                  overflow_buffer[idx].target = target;

                  PC_BTB_ENTRY tmp = overflow_buffer[idx];

                  overflow_buffer.erase(overflow_buffer.begin() + idx);
                  overflow_buffer.push_back(tmp);
                }

                if(!prefill_lookup && ret_val.first == -1){
                  assert(false);
                }
                return ret_val;
              } 
            }

            if(!found){
              num_of_misses++;

              if(prefill_lookup){
                num_of_misses_prefill++;
              }
              else{
                num_of_misses_demand++;
                if(direction){
                  btb_miss_stalls++;

                  num_of_updates++;
                  if(table[s][i].branches.size() < bundle_size){
                    table[s][i].branches.push_back(PC_BTB_ENTRY(addr, target, branch_type));
                  }
                  else{
                    overflow_buffer.erase(overflow_buffer.begin());
                    overflow_buffer.push_back(PC_BTB_ENTRY(addr, target, branch_type));
                  }
                  ret_val = make_pair(1, addr+4);
                }
                else{
                  btb_miss_not_stalls++;
                  ret_val = make_pair(0, addr+4);
                }
              }
              return ret_val;
            }
          }
        }
      }

      num_of_misses++;

      if(prefill_lookup){
        num_of_misses_prefill++;
      }
      else{
        num_of_misses_demand++;
        if(direction){
          btb_miss_stalls++;
          ret_val = make_pair(1, addr+4);
        }
        else{
          btb_miss_not_stalls++;
          ret_val = make_pair(0, addr+4);
        }
      }

      return ret_val;
    }

    void prefill(uint64_t addr, uint32_t way){
      if(!prefill_enable){
        return;
      }

      num_of_predecodes++;
      num_of_updates++;

      addr = ((addr >> 6) << 6);
      Confluence_BTB_entry tmp;

      for(uint64_t i = addr; i < (addr + 64); i += 4){
        if(perfect_PC_BTB.find(i) != perfect_PC_BTB.end()){
          uint8_t bt = perfect_PC_BTB[i].branch_type;
          if(!(bt == BRANCH_INDIRECT || bt == BRANCH_INDIRECT_CALL)){
            if(tmp.branches.size() < bundle_size){
              tmp.branches.push_back(perfect_PC_BTB[i]);
            }
            else{
              overflow_buffer.erase(overflow_buffer.begin());
              overflow_buffer.push_back(perfect_PC_BTB[i]);
            }
          }
        }
      }

      uint64_t ss = (addr >> 6) % num_of_sets;
      uint64_t tt = (addr >> 6) >> num_of_sets_bits;

      tmp.tag = tt;
      table[ss][way] = tmp;
    } 

    void printStat(){
      cout << "SHEET\n";
      cout << name << ", num_of_lookups: " << num_of_lookups << "\n";
      cout << name << ", num_of_updates: " << num_of_updates << "\n";
      cout << name << ", num_of_predecodes: " << num_of_predecodes << "\n";
      cout << name << ", num_of_hits: " << num_of_hits << "\n";
      cout << name << ", num_of_misses: " << num_of_misses << "\n";
      cout << name << ", hit rate: " << (double)num_of_hits/(num_of_hits+num_of_misses) << "\n";
      cout << name << ", num_of_hits_demand: " << num_of_hits_demand << "\n";
      cout << name << ", num_of_misses_demand: " << num_of_misses_demand << "\n";
      cout << name << ", hit rate demand: " << (double)num_of_hits_demand/(num_of_hits_demand+num_of_misses_demand) << "\n";
      cout << name << ", num_of_hits_prefill: " << num_of_hits_prefill << "\n";
      cout << name << ", num_of_misses_prefill: " << num_of_misses_prefill << "\n";
      cout << name << ", hit rate prefill: " << (double)num_of_hits_prefill/(num_of_hits_prefill+num_of_misses_prefill) << "\n";
      cout << name << ", num_of_hits_wrong_path: " << num_of_hits_wrong_path << "\n";
      cout << name << ", num_of_misses_wrong_path: " << num_of_misses_wrong_path << "\n";
      cout << name << ", btb_miss_stalls: " << btb_miss_stalls << "\n";
      cout << name << ", btb_miss_not_stalls: " << btb_miss_not_stalls << "\n";
      cout << name << ", direction_mispredictions_do_not_stall: " << direction_mispredictions_do_not_stall << "\n";   
      cout << name << ", branch_direction_stalls: " << branch_direction_stalls << "\n";
      cout << name << ", target_prediction_stalls: " << target_prediction_stalls << "\n";
      for(int i = 0; i < 9; i++){
        cout << name << ", target misprediction reason " << branch_name(i) << ": " << target_misprediction_reason[i] << "\n";
      }
      for(int i = 0; i < 9; i++){
        cout << name << ", target match reason " << branch_name(i) << ": " << target_match_reason[i] << "\n";
      }

      cout << "\n\n\n"; 
    }

    void printStalls(O3_CPU *core){
      cout << name << " btb_miss_stalls: " << (1000.0*btb_miss_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " branch_direction_stalls: " << (1000.0*branch_direction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " target_prediction_stalls: " << (1000.0*target_prediction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
    }


    void reset(){
      num_of_lookups = 0;
      num_of_updates = 0;
      num_of_predecodes = 0;

      num_of_hits = 0;
      num_of_misses  = 0;

      num_of_hits_demand = 0;
      num_of_misses_demand  = 0;

      num_of_hits_prefill = 0;
      num_of_misses_prefill  = 0;

      btb_miss_stalls = 0;
      btb_miss_not_stalls = 0;

      branch_direction_stalls = 0;
      target_prediction_stalls = 0;

      direction_mispredictions_do_not_stall = 0;

      for(int i = 0; i < 9; i++){
        target_misprediction_reason[i] = 0;
        target_match_reason[i] = 0;
      }
    }

};


class SN4L_BTB{
  public:
    struct Confluence_BTB_entry{
      uint64_t tag;
      vector<PC_BTB_ENTRY> branches;
    };

    uint64_t num_of_sets;
    uint64_t num_of_ways;

    uint64_t num_of_sets_pb;
    uint64_t num_of_ways_pb;


    uint64_t num_of_sets_bits;
    uint64_t num_of_sets_pb_bits;


    uint64_t num_of_lookups = 0;
    uint64_t num_of_updates = 0;
    uint64_t num_of_predecodes = 0;
    uint64_t num_of_updates_pb = 0;

    uint64_t num_of_hits = 0;
    uint64_t num_of_misses  = 0;

    uint64_t num_of_hits_demand = 0;
    uint64_t num_of_misses_demand  = 0;

    uint64_t num_of_hits_prefill = 0;
    uint64_t num_of_misses_prefill  = 0;

    uint64_t num_of_hits_wrong_path = 0;
    uint64_t num_of_misses_wrong_path = 0;

    uint64_t btb_miss_stalls = 0;
    uint64_t btb_miss_not_stalls = 0;

    uint64_t branch_direction_stalls = 0;
    uint64_t target_prediction_stalls = 0;

    uint64_t direction_mispredictions_do_not_stall = 0;

    vector< vector<PC_BTB_ENTRY> > table;
    vector< vector<Confluence_BTB_entry> > prefetch_buffer;
    vector< uint64_t> RAS;

    uint64_t target_misprediction_reason[9] = {0};
    uint64_t target_match_reason[9] = {0};

    bool prefill_enable;
    string name;

    uint32_t bundle_size = 3;

    SN4L_BTB(uint64_t s, uint64_t w, uint64_t s_pb, uint64_t w_pb, bool pe, string n){
      prefill_enable = pe;

      name = n;

      num_of_sets = s;
      num_of_ways = w;

      num_of_sets_pb = s_pb;
      num_of_ways_pb = w_pb;

      num_of_sets_bits = log2(num_of_sets);
      num_of_sets_pb_bits = log2(num_of_sets_pb);

      table.resize(num_of_sets);
      for(uint i = 0; i < num_of_sets; i++){
        table[i].resize(num_of_ways);
      }

      prefetch_buffer.resize(num_of_sets_pb);
      for(uint i = 0; i < num_of_sets_pb; i++){
        prefetch_buffer[i].resize(num_of_ways_pb);
      }
    }

    uint64_t flookup(uint64_t addr, bool isBranch){
      num_of_lookups++;

      if(!isBranch){
        return 0;
      }

      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          num_of_hits++;
          num_of_hits_wrong_path++;

          uint64_t target = table[s][i].target;

          PC_BTB_ENTRY tmp = table[s][i];
          table[s].erase(table[s].begin() + i);
          table[s].push_back(tmp);
          return target;
        }
      }

      PC_BTB_ENTRY pb = pb_get_entry(addr);
       
      if(pb.target != 0){
        num_of_hits++;
        num_of_hits_wrong_path++;
        move_from_pb_to_btb(pb);
        return pb.target;
      }

      num_of_misses++;
      num_of_misses_wrong_path++;
      return addr + 4;
    }

    PC_BTB_ENTRY pb_get_entry(uint64_t addr){
      uint64_t s = (addr >> 6) % num_of_sets_pb;
      uint64_t t = (addr >> 6) >> num_of_sets_pb_bits;

      for(uint i = 0; i < num_of_ways_pb; i++){
        if(prefetch_buffer[s][i].tag == t){
          for(uint j = 0; j < prefetch_buffer[s][i].branches.size(); j++){
            if(prefetch_buffer[s][i].branches[j].tag == addr){

              PC_BTB_ENTRY ret_val = prefetch_buffer[s][i].branches[j];

              Confluence_BTB_entry tmp = prefetch_buffer[s][i];
              prefetch_buffer[s].erase(prefetch_buffer[s].begin()+i);
              prefetch_buffer[s].push_back(tmp);

              return ret_val;
            }
          }
        }
      }

      return PC_BTB_ENTRY();
    }

    PC_BTB_ENTRY get_entry(uint64_t addr){
      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          return table[s][i];
        }
      }

      return PC_BTB_ENTRY();
    }


    void move_from_pb_to_btb(PC_BTB_ENTRY e){
      uint64_t s = (e.tag >> 2) % num_of_sets;
      uint64_t t = (e.tag >> 2) >> num_of_sets_bits;

      e.tag = t;

      table[s].erase(table[s].begin());
      table[s].push_back(e);
      num_of_updates++;
    }

    pair<int, uint64_t> lookup(uint64_t addr, bool prefill_lookup = false, uint8_t direction = 0, uint8_t pDirection = 0, uint64_t target = 0, uint8_t branch_type = 0){
      pair<int, uint64_t> ret_val = make_pair(-1, 0);

      if(prefill_lookup){
        assert(false);
      }

      num_of_lookups++;

      if(branch_type == NOT_BRANCH){
        return ret_val;
      } 

      uint64_t pTarget = addr + 4;

      if(!prefill_lookup){
        if(branch_type == BRANCH_INDIRECT_CALL || branch_type == BRANCH_DIRECT_CALL){
          RAS.push_back(addr + 4);
        }
        else if(branch_type == BRANCH_RETURN && !RAS.empty()){
          pTarget = RAS.back();
          RAS.erase(RAS.begin() + RAS.size() - 1);
        }
      }


      uint64_t s = (addr >> 2) % num_of_sets;
      uint64_t t = (addr >> 2) >> num_of_sets_bits;

      for(uint i = 0; i < num_of_ways; i++){
        if(table[s][i].tag == t){
          num_of_hits++;
          if(prefill_lookup){
            num_of_hits_prefill++;
          }
          else{
            num_of_hits_demand++;
            if(branch_type == BRANCH_RETURN && !RAS.empty() && pDirection){
            }
            else{
              if(pDirection){
                pTarget = table[s][i].target;
              }
              else{
                pTarget = addr + 4;
              }
            }

            if(direction == pDirection){ 
              if(pTarget == target){
                target_match_reason[branch_type]++;
                ret_val = make_pair(0, pTarget);
              }
              else{
                ret_val = make_pair(6, pTarget);
                target_misprediction_reason[branch_type]++;
                target_prediction_stalls++;
              }
            }
            else{
              if(!direction && pDirection && table[s][i].target == (addr + 4)){
                direction_mispredictions_do_not_stall++;
                ret_val = make_pair(0, pTarget);
              }
              else{
                ret_val = make_pair(4, pTarget);
                branch_direction_stalls++;
              }
            }
          }

          if(prefill_lookup){
            num_of_updates++;
          }
          else{
            if(direction){
              if(pTarget == target){
                return ret_val;
              }
              else{
                if(pTarget != target){
                  if(branch_type != BRANCH_RETURN){
                    num_of_updates++;
                  }
                  else{
                    if(pTarget == (addr + 4)){
                      num_of_updates++;
                    }
                    else{
                      return ret_val;
                    }
                  }
                }
                else{
                  return ret_val;
                }
              }
            }
            else{
              return ret_val;
            }
          }


          PC_BTB_ENTRY tmp = table[s][i];
          if(direction){
            tmp.target = target;
          }
          table[s].erase(table[s].begin()+i);
          table[s].push_back(tmp);

          if(!prefill_lookup && ret_val.first == -1){
            assert(false);
          }

          return ret_val;
        }
      }

      PC_BTB_ENTRY pb = pb_get_entry(addr);
      if(pb.target != 0){   
        num_of_hits++;
        if(prefill_lookup){
          num_of_hits_prefill++;
        }
        else{
          num_of_hits_demand++;
          if(branch_type == BRANCH_RETURN && !RAS.empty() && pDirection){
          }
          else{
            if(pDirection){
              pTarget = pb.target;
            }
            else{
              pTarget = addr + 4;
            }
          }

          if(direction == pDirection){ 
            if(pTarget == target){
              target_match_reason[branch_type]++;
              ret_val = make_pair(0, pTarget);
            }
            else{
              ret_val = make_pair(6, pTarget);
              target_misprediction_reason[branch_type]++;
              target_prediction_stalls++;
            }
          }
          else{
            if(!direction && pDirection && pb.target == (addr + 4)){
              direction_mispredictions_do_not_stall++;
              ret_val = make_pair(0, pTarget);
            }
            else{
              ret_val = make_pair(4, pTarget);
              branch_direction_stalls++;
            }
          }
        }

        if(prefill_lookup){
          num_of_updates++;
        }
        else{
          if(direction){
            if(pTarget == target){
            }
            else{
              if(pTarget != target){
                if(branch_type != BRANCH_RETURN){
                  pb.target = target;
                  num_of_updates++;
                }
                else{
                  if(pTarget == (addr + 4)){
                    pb.target = target;
                    num_of_updates++;
                  }
                  else{
                  }
                }
              }
              else{
              }
            }
          }
          else{
          }
        }
        move_from_pb_to_btb(pb);
        return ret_val;
      }

      num_of_misses++;
      if(prefill_lookup){
        num_of_misses_prefill++;

        num_of_updates++;

        // prefilling mechanism may insert all branches!
        table[s].erase(table[s].begin());
        table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));
      }
      else{
        num_of_misses_demand++;
        if(direction){
          btb_miss_stalls++;

          num_of_updates++;

          table[s].erase(table[s].begin());
          table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));
          ret_val = make_pair(1, addr+4);
        }
        else{
          btb_miss_not_stalls++;

          // Our BTB inserts an entry if it finds the missing branch is taken. 
          // If you want to insert also not taken branches, uncomment the following lines
          //num_of_updates++;
          //table[s].erase(table[s].begin());
          //table[s].push_back(PC_BTB_ENTRY(t, target, branch_type));

          ret_val = make_pair(0, addr+4);
        }
      }

      return ret_val;
    }

    void prefill(uint64_t addr){
      if(!prefill_enable){
        return;
      }

      num_of_predecodes++;

      addr = ((addr >> 6) << 6);
      Confluence_BTB_entry tmp;

      for(uint64_t i = addr; i < (addr + 64); i += 4){
        if(perfect_PC_BTB.find(i) != perfect_PC_BTB.end()){
          uint8_t bt = perfect_PC_BTB[i].branch_type;
          if(!(bt == BRANCH_INDIRECT || bt == BRANCH_INDIRECT_CALL)){
            if(tmp.branches.size() < bundle_size){
              tmp.branches.push_back(perfect_PC_BTB[i]);
            }
          }
        }
      }

      if(tmp.branches.size() == 0){
        return;
      }

      num_of_updates_pb++;

      uint64_t ss = (addr >> 6) % num_of_sets_pb;
      uint64_t tt = (addr >> 6) >> num_of_sets_pb_bits;

      tmp.tag = tt;

      for(uint i = 0; i < num_of_ways_pb; i++){
        if(prefetch_buffer[ss][i].tag == tt){
            prefetch_buffer[ss][i] = tmp;
            return;
        }
      }

      prefetch_buffer[ss].erase(prefetch_buffer[ss].begin());
      prefetch_buffer[ss].push_back(tmp);
    } 

    void printStat(){
      cout << "SHEET\n";
      cout << name << ", num_of_lookups: " << num_of_lookups << "\n";
      cout << name << ", num_of_updates: " << num_of_updates << "\n";
      cout << name << ", num_of_predecodes: " << num_of_predecodes << "\n";
      cout << name << ", num_of_updates_pb: " << num_of_updates_pb << "\n";
      cout << name << ", num_of_hits: " << num_of_hits << "\n";
      cout << name << ", num_of_misses: " << num_of_misses << "\n";
      cout << name << ", hit rate: " << (double)num_of_hits/(num_of_hits+num_of_misses) << "\n";
      cout << name << ", num_of_hits_demand: " << num_of_hits_demand << "\n";
      cout << name << ", num_of_misses_demand: " << num_of_misses_demand << "\n";
      cout << name << ", hit rate demand: " << (double)num_of_hits_demand/(num_of_hits_demand+num_of_misses_demand) << "\n";
      cout << name << ", num_of_hits_prefill: " << num_of_hits_prefill << "\n";
      cout << name << ", num_of_misses_prefill: " << num_of_misses_prefill << "\n";
      cout << name << ", hit rate prefill: " << (double)num_of_hits_prefill/(num_of_hits_prefill+num_of_misses_prefill) << "\n";
      cout << name << ", num_of_hits_wrong_path: " << num_of_hits_wrong_path << "\n";
      cout << name << ", num_of_misses_wrong_path: " << num_of_misses_wrong_path << "\n";
      cout << name << ", btb_miss_stalls: " << btb_miss_stalls << "\n";
      cout << name << ", btb_miss_not_stalls: " << btb_miss_not_stalls << "\n";
      cout << name << ", direction_mispredictions_do_not_stall: " << direction_mispredictions_do_not_stall << "\n";   
      cout << name << ", branch_direction_stalls: " << branch_direction_stalls << "\n";
      cout << name << ", target_prediction_stalls: " << target_prediction_stalls << "\n";
      for(int i = 0; i < 9; i++){
        cout << name << ", target misprediction reason " << branch_name(i) << ": " << target_misprediction_reason[i] << "\n";
      }
      for(int i = 0; i < 9; i++){
        cout << name << ", target match reason " << branch_name(i) << ": " << target_match_reason[i] << "\n";
      }

      cout << "\n\n\n"; 
    }

    void printStalls(O3_CPU *core){
      cout << name << " btb_miss_stalls: " << (1000.0*btb_miss_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " branch_direction_stalls: " << (1000.0*branch_direction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
      cout << name << " target_prediction_stalls: " << (1000.0*target_prediction_stalls)/(core->num_retired - core->warmup_instructions) << "\n";
    }


    void reset(){
      num_of_lookups = 0;
      num_of_updates = 0;
      num_of_predecodes = 0;
      num_of_updates_pb = 0;

      num_of_hits = 0;
      num_of_misses  = 0;

      num_of_hits_demand = 0;
      num_of_misses_demand  = 0;

      num_of_hits_prefill = 0;
      num_of_misses_prefill  = 0;

      btb_miss_stalls = 0;
      btb_miss_not_stalls = 0;

      branch_direction_stalls = 0;
      target_prediction_stalls = 0;

      direction_mispredictions_do_not_stall = 0;

      for(int i = 0; i < 9; i++){
        target_misprediction_reason[i] = 0;
        target_match_reason[i] = 0;
      }
    }

};



PC_BTB pc_btb_2K_wo(256, 8, false, "2K_wo_prefill");
PC_BTB pc_btb_2K_w(256, 8, true,  "2K_with_prefill");
PC_BTB pc_btb_4K_wo(512, 8, false, "4K_wo_prefill");
PC_BTB pc_btb_4K_w(512, 8, true,  "4K_with_prefill");
PC_BTB pc_btb_8K_wo(1024, 8, false, "8K_wo_prefill");
PC_BTB pc_btb_8K_w(1024, 8, true,  "8K_with_prefill");
PC_BTB pc_btb_16K_wo(2048, 8, false, "16K_wo_prefill");
PC_BTB pc_btb_16K_w(2048, 8, true,  "16K_with_prefill");
PC_BTB pc_btb_32K_wo(4096, 8, false, "32K_wo_prefill");
PC_BTB pc_btb_32K_w(4096, 8, true,  "32K_with_prefill");



Confluence_BTB cnf_btb(L1I_SET, L1I_WAY, true, "Confluence_BTB");

SN4L_BTB sn4l_btb_2K(258, 8, 32, 2, true, "2K_sn4l_BTB");
