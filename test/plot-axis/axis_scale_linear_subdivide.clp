(layer/resize 1024px 512px)
(layer/set-dpi 96)

(tools/plotgen
    margin 8em
    axis (
        align x
        label-placement (subdivide 21)
        limit (-100 100)))
