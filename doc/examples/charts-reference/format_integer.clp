(layer/resize 1024px 60px)
(layer/set-dpi 96)

(tools/plotgen
    margin 1em
    axis (
        label-format (integer)
        label-placement (linear 1)
        limit (1 16)))
