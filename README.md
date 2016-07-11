# dlt_poc
DltInc_wrapper class contains public usable add node/remove node apis.
DltInc class implements the actual token distribution and add/remove node logic.

Apart from that, please refer to the note below:

  There is a strong correlation between the total number of tokens and
  total number of nodes a cluster should have.
 
  Taking the standard FDS configuration as benchmark where there are
  10 disks in a node, for parallelism each disk should hold atleast
  1 primary token, so for any given token distribution, there
  should be atleast 10 primary tokens assigned to a SM.
  If the number of primary tokens assigned to a SM in cluster falls
  below 10, that is an indication that we need more granular tokens/
  more number of tokens for a given deployment.
 
  The optimum configurations:
  Num of Tokens ---> maximum number of SMs a cluster should have.
   256 tokens   --->    ~24 nodes
   512 tokens   --->   ~48 nodes
  and so on.
 
 
 
