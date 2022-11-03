// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/**
 * queue_synch - searchers in input buffer for all instances of "search" and
 * repalces them with "replacement" input - input buffer size - buffer size
 */
__kernel void queue_synch(__global int *input, int search, int replacement,
                          int input_size) {
  int workItemSize = input_size / get_global_size(0);
  int firstIndex = get_global_id(0) * workItemSize;
  int lastIndex = firstIndex + workItemSize;
  for (int i = firstIndex; i < lastIndex; ++i) {
    if (search == input[i]) {
      input[i] = replacement;
    }
  }
}

/**
 * shared_synch - determines which devies this kernel is being run on (with use
 *of global size - global size greater than 1 is for GPU, otherwise - CPU). If
 *its being run for CPU - works on lower half of the buffer, otherwise on higher
 *half of the buffer. Searchers in input buffer for all instances of "search"
 *and repalces them with "replacement" input - input buffer size - buffer size
 */
__kernel void shared_synch(__global int *input, int search, int replacement,
                           int input_size) {
  if (get_global_size(0) > 1) {
    // this is GPU
    // write to higher half of array
    for (int i = input_size; i < 2 * input_size; ++i) {
      if (search == input[i]) {
        input[i] = replacement;
      }
    }
    return;
  }
  // this is CPU
  // write to lower half
  for (int i = 0; i < input_size; ++i) {
    if (search == input[i]) {
      input[i] = replacement;
    }
  }
}
