### $Id: BOD.R,v 1.2 2005/08/15 16:11:26 johnmc Exp $
### Biochemical Oxygen Demand data from Marske
"BOD" <-
  structure(list(Time = c(1, 2, 3, 4, 5, 7),
                 demand = c(8.3, 10.3, 19, 16, 15.6, 19.8)),
            .Names = c("Time", "demand"),
            row.names = c("1", "2", "3", "4", "5", "6"),
            class = "data.frame",
            reference = "A1.4, p. 270")
