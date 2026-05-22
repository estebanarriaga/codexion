t_request represents one coder waiting for one dongle.

  typedef struct s_request
  {
  	int		coder_id;
  	long	arrival_ms;
  	long	deadline_ms;
  	long	sequence;
  }	t_request;

  It is needed because the subject says dongles cannot be granted randomly. If several coders
  want the same dongle, the program must choose according to the selected scheduler:

  fifo -> coder whose request arrived first
  edf  -> coder whose burnout deadline is earliest

  So the dongle needs memory of each waiting coder. That memory is t_request.

  Field meaning:

  int coder_id;

  Which coder is waiting.

  long arrival_ms;

  When this request entered the dongle’s wait queue. FIFO uses this.

  long deadline_ms;

  When this coder will burn out if it does not start compiling. EDF uses this.

  long sequence;

  A tie-breaker. If two requests have the same arrival time or same deadline, sequence gives a
  deterministic order.

  Example:

  request A: coder 2, arrival 100ms, deadline 900ms
  request B: coder 3, arrival 120ms, deadline 700ms

  With fifo, coder 2 wins because 100ms < 120ms.

  With edf, coder 3 wins because 700ms < 900ms.

  Without t_request, the program only knows “some coders are waiting.” It does not know enough to
  choose fairly or correctly.
