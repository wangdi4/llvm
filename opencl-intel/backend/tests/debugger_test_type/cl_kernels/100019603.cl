// repro from CrowdSim sample
struct __Agent;

typedef struct __Obstacle {
  __global struct __Obstacle *nextObstacle_;
  __global struct __Obstacle *prevObstacle_;
} Obstacle;

typedef struct __AgentNeighbor {
  __global const struct __Agent *second;
} AgentNeighbor;

typedef struct __ObstacleNeighbor {
  __global const struct __Obstacle *second;
} ObstacleNeighbor;

typedef struct __Agent {
  __global AgentNeighbor *agentNeighbors_;
  __global ObstacleNeighbor *obstacleNeighbors_;
} Agent;

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  __global Agent *agent = (__global Agent *)buf_in;
  __global Agent *agent_out = (__global Agent *)buf_out;
  agent_out->agentNeighbors_ = agent->agentNeighbors_;
  return;
}
