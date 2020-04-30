/*
  ==============================================================================
  ==============================================================================
  stats.h

  Note that this starts a thread to periodically poll /proc/stats.
  ==============================================================================
  ==============================================================================
*/

#ifndef _STATS_H_
#define _STATS_H_

int stats_initialize(void);
void stats_finalize(void);

int get_temp(void);
long get_load(void);

#endif	/* _STATS_H_ */
