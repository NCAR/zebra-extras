;GIL-2
(
(
	:type                   :base-window
	:name                   basewindow
	:owner                  nil
	:width                  357
	:height                 111
	:background-color       ""
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:mapped                 t
	:show-footer            nil
	:resizable              nil
	:icon-file              ""
	:icon-mask-file         ""
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls2
	:owner                  basewindow
	:help                   ""
	:x                      0
	:y                      0
	:width                  357
	:height                 111
	:background-color       "Black"
	:foreground-color       "Cyan"
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   label
	:owner                  controls2
	:help                   ""
	:x                      64
	:y                      52
	:width                  223
	:height                 13
	:foreground-color       "Cyan"
	:label                  "prds, the product selector, starts"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :popup-window
	:name                   popup1
	:owner                  basewindow
	:width                  500
	:height                 230
	:background-color       ""
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:mapped                 nil
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          popup1_e
	:events                 (:mouse-exit )
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  popup1
	:help                   ""
	:x                      0
	:y                      0
	:width                  500
	:height                 230
	:background-color       "LIght gray"
	:foreground-color       "Black"
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :scrolling-list
	:name                   prd_list
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:width                  160
	:height                 217
	:foreground-color       "black"
	:label                  "Overlay products"
	:label-type             :string
	:layout-type            :vertical
	:rows                   10
	:read-only              t
	:multiple-selections    t
	:selection-required     t
	:menu                   nil
	:notify-handler         prd_list_h
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :scrolling-list
	:name                   ind_list
	:owner                  controls1
	:help                   ""
	:x                      192
	:y                      8
	:width                  140
	:height                 217
	:foreground-color       "black"
	:label                  "ind_list"
	:label-type             :string
	:layout-type            :vertical
	:rows                   10
	:read-only              t
	:multiple-selections    t
	:selection-required     t
	:menu                   nil
	:notify-handler         ind_list_h
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :scrolling-list
	:name                   tim_list
	:owner                  controls1
	:help                   ""
	:x                      356
	:y                      8
	:width                  120
	:height                 181
	:foreground-color       "Black"
	:label                  "tim_list"
	:label-type             :string
	:layout-type            :vertical
	:rows                   8
	:read-only              t
	:multiple-selections    t
	:selection-required     t
	:menu                   nil
	:notify-handler         tim_list_h
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   cut
	:owner                  controls1
	:help                   ""
	:x                      364
	:y                      196
	:width                  55
	:height                 23
	:value-x                373
	:value-y                196
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:rows                   1
	:columns                0
	:label                  ""
	:label-type             :string
	:notify-handler         cut_h
	:event-handler          nil
	:events                 ()
	:choices                (" Cut " )
	:choice-label-types     (:string )
	:choice-colors          ("Black" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   loop
	:owner                  controls1
	:help                   ""
	:x                      428
	:y                      196
	:width                  53
	:height                 23
	:value-x                437
	:value-y                196
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:rows                   1
	:columns                0
	:label                  ""
	:label-type             :string
	:notify-handler         loop_h
	:event-handler          nil
	:events                 ()
	:choices                ("Loop" )
	:choice-label-types     (:string )
	:choice-colors          ("Black" )
	:user-data              ()
)
)
