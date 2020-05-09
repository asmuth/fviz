(layer/resize 1024px 512px)
(layer/set-dpi 96)

(tools/plotgen
    margin 8em
    axis (
        align top
        limit (1451606400 1451610000)
        label-format (datetime "%H:%M:%S")))
