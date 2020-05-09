(layer/resize 1024px 512px)
(layer/set-dpi 96)

(tools/plotgen
    margin 8em
    axis (
        align top
        label-placement (linear 1)
        limit (0 16)
        title "Fnord (f/s)"
        title-rotate 45
        title-font-size 16pt))
